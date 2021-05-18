#include "so_scheduler.h"
#include "thread_util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define SO_VERBOSE_ERROR

#define BARRIER_MAX_THREADS 2

static TScheduler *scheduler = NULL;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ thread_sync ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void signal_finish()
{
	scheduler->finish_flag = 1;
	pthread_cond_signal(&(scheduler->finish_cond_var));
	printf("(signal_finish)(am dat signal de finish)\n");
}

static void wait_to_finish()
{
	pthread_mutex_lock(&(scheduler->lock));

	while (scheduler->finish_flag != 1) {
		printf("(wait_to_finish)(asteptam signal de finish)\n");
		pthread_cond_wait(&(scheduler->finish_cond_var), 
			&(scheduler->lock));
	}
	printf("(wait_to_finish)(am primit signal de finish)\n");

	pthread_mutex_unlock(&(scheduler->lock));
}

static void thread_wait_to_get_scheduled(TThread_struct *thread)
{
	pthread_mutex_lock(&(scheduler->lock));

	while (!(thread->condition)) {
		printf("THREAD [%ld] GOING TO SLEEP\n", thread->id);
		pthread_cond_wait(&(thread->cond_var), 
			&(scheduler->lock));
	}

	pthread_mutex_unlock(&(scheduler->lock));
}

static void signal_thread_scheduled(TThread_struct *thread)
{
	pthread_mutex_lock(&(scheduler->lock));

	thread->condition = 0;

	pthread_mutex_unlock(&(scheduler->lock));
}

static void signal_parent_thread_of_forked()
{
	pthread_mutex_lock(&(scheduler->fork_lock));
	scheduler->fork_flag = 1;
	pthread_cond_signal(&(scheduler->fork_cond_var));
	pthread_mutex_unlock(&(scheduler->fork_lock));
}

static void wait_for_forked_thread()
{
	printf("(wait_for_forked_thread)\n");

	pthread_mutex_lock(&(scheduler->fork_lock));
	printf("(wait_for_forked_thread)(fork_lock acquired)\n");

	while(!(scheduler->fork_flag)) {
		printf("(wait_for_forked_thread)(fork_lock released)(wait for forked)\n");
		pthread_cond_wait(&(scheduler->fork_cond_var),
			&(scheduler->fork_lock));
		printf("[4](wait_for_forked_thread)(am primit signal ca putem face)"
		"check_schedule() safely\n");
	}
	
	scheduler->fork_flag = 0;

	pthread_mutex_unlock(&(scheduler->fork_lock));
	printf("(wait_for_forked_thread)(fork_lock released)\n");
}

static TThread_struct *get_thread_struct_from_ready()
{
	pthread_t id = pthread_self();
	return find_element_in_queue(&(scheduler->ready_queue),
		&id,
		cmp_find_by_id);
}

static TThread_struct *get_thread_struct_from_waiting()
{

	// TODO: fix me

	// return find_element_in_queue(&(scheduler->ready_queue),
	// 	pthread_self(),
	// 	cmp_find_by_id);
	return NULL;
}

/* newly create thread will call this before executing the handler
 * as this should wait to get a signal from the scheduler to start executing
 */
static void wait_to_start_executing()
{
	printf("(wait_to_start_executing)\n");
	TThread_struct *thread = get_thread_struct_from_ready();
	if (thread == NULL)
		printf("aici e buba\n");

	// signals parent thread it can safely call check_scheduler
	pthread_mutex_lock(&(scheduler->fork_lock));
	printf("(wait_to_start_executing)(fork_lock acquired)\n");

	scheduler->fork_flag = 1;
	pthread_cond_signal(&(scheduler->fork_cond_var));
	printf("[1](wait_to_start_executing)(fork_cond_var signal)\n");

	// locks on general lock
	pthread_mutex_lock(&(scheduler->lock));
	printf("[2](wait_to_start_executing)(lock acquired)\n");

	// releases fork lock for parent thread to take and get out of wait
	// for safety signal
	pthread_mutex_unlock(&(scheduler->fork_lock));
	printf("[3](wait_to_start_executing)(fork_lock released)\n");

	// actually waits on general lock for signal to start executing
	while (thread->condition != 1) {
		printf("THREAD [%ld] GOING TO SLEEP\n", thread->id);
		printf("(wait_to_start_executing)(wait on cond variable)\n");

		// now waits for signal to start exec
		pthread_cond_wait(&(thread->cond_var), 
			&(scheduler->lock));
		printf("(wait_to_start_executing)(release from cond variable)\n");
	}
	pthread_mutex_unlock(&(scheduler->lock));
	printf("(wait_to_start_executing)(lock released)\n");
}

static void finished_execution()
{
	TThread_struct *finished_thread = scheduler->running_thread;

	finished_thread->condition = 0;
	pthread_cond_destroy(&(finished_thread->cond_var));
	free(finished_thread);

	if (scheduler->ready_queue.size == 0) {
		//TODO: signal finish cuz there are no more threads left to run;
		printf("(finished_execution)(aici trebe signal de final)\n");
		pthread_mutex_lock(&(scheduler->lock));
		signal_finish();
		pthread_mutex_unlock(&(scheduler->lock));
		return;
	}

	TThread_struct *next_thread = dequeue_pr(&(scheduler->ready_queue));

	// set next running thread
	scheduler->running_thread = next_thread;

	// reset timer
	scheduler->crt_thread_time_quantum = 0;

	// singal next thread
	pthread_mutex_lock(&(scheduler->lock));
	next_thread->condition = 1;
	pthread_cond_signal(&(next_thread->cond_var));
	pthread_mutex_unlock(&(scheduler->lock));
}

static void check_scheduler()
{
	TThread_struct *crt_thread = get_thread_struct_from_ready();

	printf("se apeleaza (check_scheduler)\n");

	if (pthread_equal(pthread_self(), scheduler->master_thread_id)) {
		// here we should somehow start the scheduling 
		// but not block here
		printf("(check_scheduler)(before dequeue)\n");
		TThread_struct *first_thread = dequeue_pr(&(scheduler->ready_queue));
		scheduler->crt_running_thread_id = first_thread->id;
		scheduler->running_thread = first_thread;
		
		// this instructions starts the first forked thread
		pthread_mutex_lock(&(scheduler->lock));
		first_thread->condition = 1;
		pthread_cond_signal(&(first_thread->cond_var));
		printf("Starting the first forked thread\n");
		pthread_mutex_unlock(&(scheduler->lock));
		return;
	}

	pthread_mutex_lock(&(scheduler->lock));
	scheduler->crt_thread_time_quantum++;
	printf("(check_scheduler)(a crescut crt_running_time = [%d])\n",
		scheduler->crt_thread_time_quantum);

	if (scheduler->crt_thread_time_quantum == scheduler->max_time_quantum) {
		printf("\t(check_scheduler)(max_quantum reached)\n");

		// add running back to ready (if not so_wait)
		printf("\t\t(check_scheduler)(max_quantum)\n");
		TThread_struct *old_running = scheduler->running_thread;

		// reset condition
		old_running->condition = 0;
		
		// add the old running back to ready q
		add_to_queue(&(scheduler->ready_queue),
			old_running,
			cmp_by_priority);
		
		// deq from ready q
		TThread_struct *new_running =
			dequeue_pr(&(scheduler->ready_queue));
		
		// set new running
		scheduler->running_thread = new_running;
		scheduler->crt_thread_time_quantum = 0;

		// singnal new running thread
		new_running->condition = 1;
		pthread_cond_signal(&(new_running->cond_var));
		
		// sleep old thread
		while (old_running->condition != 1) {
			pthread_cond_wait(&(old_running->cond_var),
				&(scheduler->lock));
		}

		// old thread resumed execution
		// TODO2: at this point the thread could have finished
		// all instructions but we don't know that yet
		pthread_mutex_unlock(&(scheduler->lock));
		printf("\t\t(check_scheduler)(max_quantum)(resume)\n");

	} else {
		printf("\t(check_scheduler)(peek)\n");
		// momentan nu luam in calcul ca instructiunea a fost un wait
		// si trebuie evacuat baiatul din runnning/ready
		// dar cred ca evacuarea se face in functia de wait
		TThread_struct *head = peek(&(scheduler->ready_queue));

		// higher priority thread is present, crt running has to be
		// preempted
		if (head != NULL &&
			head->priority > scheduler->running_thread->priority) {
			
			printf("\t\t(check_scheduler)(peek)(higher priority)\n");
			TThread_struct *old_running = scheduler->running_thread;
			
			// reset condition
			old_running->condition = 0;
			
			// deq from ready q
			TThread_struct *new_running =
				dequeue_pr(&(scheduler->ready_queue));
			
			// add the old running back to ready q
			add_to_queue(&(scheduler->ready_queue),
				old_running,
				cmp_by_priority);
			
			// set new running
			scheduler->running_thread = new_running;
			scheduler->crt_thread_time_quantum = 0;

			// singnal new running thread
			new_running->condition = 1;
			pthread_cond_signal(&(new_running->cond_var));
			
			// sleep old thread
			while (old_running->condition != 1) {
				pthread_cond_wait(&(old_running->cond_var),
					&(scheduler->lock));
			}

			// old thread resumed execution
			// TODO2: at this point the thread could have finished
			// all instructions but we don't know that yet
			pthread_mutex_unlock(&(scheduler->lock));
			printf("\t\t(check_scheduler)(peek)(higher priority)(resume)\n");
			return;
		} else {
			// continue running the current thread
			// check for finish?
			printf("\t\t(check_scheduler)(peek)(NO higher priority)\n");
			pthread_mutex_unlock(&(scheduler->lock));
		}
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~ aux_functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int is_master_thread()
{
	return pthread_equal(scheduler->master_thread_id, pthread_self());
}

static TThread_struct *new_thread_struct(tid_t tid, unsigned int priority)
{
	TThread_struct *instance = calloc(1, sizeof(TThread_struct));
	if (instance == NULL)
		return NULL;
	
	instance->id = tid;
	instance->priority = priority;
	pthread_cond_init(&(instance->cond_var), NULL);

	return instance;
}
 

static void *start_function(void *arg)
{
	TPthread_arg args = *(TPthread_arg *)arg;

	// arg is allocated just before calling this function:
	// once we get here we have the copy of arg locally stored
	free(arg);

	/* TODO: some sort of signal to parent thread that we are ready to
	 * run; parent thread waits until we get this far.
	 */

	/* TODO: some sort of lock because the thread will immediately reach
	 * this point; here we wait (somehow) to te planned on CPU
	 * wait to be added to ready queue
	 */
	pthread_barrier_wait(&(scheduler->fork_barrier));

	// wait to get scheduled ???
	// this 
	wait_to_start_executing();

	args.handler(args.priority);

	// thread finished executing -> delete from ready/running??
	printf("(start_function)(finished execution)\n");
	// TODO2: here we know the thread finished execution so we might
	// call check_scheduler with a flag of termination
	finished_execution();

	pthread_exit(NULL);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~ header_functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int so_init(unsigned int time_quantum, unsigned int io)
{
	// int rc = 0;

	if (scheduler != NULL) {
		return -1;
	}

	if (io > SO_MAX_NUM_EVENTS) {
		return -1;
	}

	if (time_quantum == 0)
		return -1;

	scheduler = calloc(1, sizeof(TScheduler));
	if (scheduler == NULL)
		return -1;

	scheduler->io_events = io;
	scheduler->max_time_quantum = time_quantum;
	scheduler->crt_thread_time_quantum = 0;
	scheduler->crt_running_thread_id = INVALID_TID;

	pthread_mutex_init(&(scheduler->lock), NULL);
	pthread_mutex_init(&(scheduler->fork_lock), NULL);
	pthread_cond_init(&(scheduler->fork_cond_var), NULL);
	pthread_cond_init(&(scheduler->finish_cond_var), NULL);
	pthread_barrier_init(&(scheduler->fork_barrier), NULL,
		BARRIER_MAX_THREADS);
	scheduler->fork_flag = 0;
	scheduler->finish_flag = 0;

	scheduler->master_thread_id = pthread_self();
	init_queue(&(scheduler->ready_queue));
	scheduler->thread_id_list = NULL;

	for (int i = 0; i < io; i++) {
		scheduler->waiting_lists[i] = NULL;
	}

	return 0;

// err_free:
// 	free(scheduler);
// 	scheduler = NULL;
// 	return -1;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	if (func == NULL || priority > SO_MAX_PRIO)
		return INVALID_TID;

	TPthread_arg *arg = calloc(1, sizeof(TPthread_arg));
	if (arg == NULL)
		return INVALID_TID;

	/* this is do-work part */
	arg->handler = func;
	arg->priority = priority;
	arg->is_master = is_master_thread();

	pthread_t new_thread = INVALID_TID;
	pthread_create(&new_thread, NULL, start_function, arg);

	TThread_struct *t_struct = new_thread_struct(new_thread, priority);
	if (t_struct == NULL)
		return INVALID_TID;

	pthread_t *new_thread_copy = calloc (1, sizeof(pthread_t));
	if (new_thread_copy == NULL)
		return INVALID_TID;
	
	*new_thread_copy = new_thread;
	add_element_to_front(&(scheduler->thread_id_list), new_thread_copy);

	/* here we wait to get signaled by the just create thread that it's
	 * ready to run before we continue;
	 * we do this with some mutex I think;
	 */

	printf("am ajuns aici\n");
	add_to_queue(&(scheduler->ready_queue), t_struct, cmp_by_priority);
	// add_to_queue(&(scheduler->ready_queue), t_struct, cmp_by_priority_2);
	pthread_barrier_wait(&(scheduler->fork_barrier));
	wait_for_forked_thread();

	/* this is check scheduler part */
	check_scheduler();
	

	return new_thread;
}

int so_wait(unsigned int io)
{
	if (io >= scheduler->io_events)
		return -1;

	// move the currently running thread's struct into the waiting list
	TThread_struct *old_running = scheduler->running_thread;
	add_element_to_front(&(scheduler->waiting_lists[io]), old_running);

	// clear the currently running thread info
	scheduler->running_thread = NULL;
	scheduler->crt_thread_time_quantum = 0;

	// clear info about old thread
	old_running->condition = 0;

	TThread_struct *new_running = dequeue_pr(&(scheduler->ready_queue));
	
	scheduler->running_thread = new_running;

	// signal new thread
	pthread_mutex_lock(&(scheduler->lock));
	new_running->condition = 1;
	pthread_cond_signal(&(new_running->cond_var));

	// sleep old thread
	while (old_running->condition != 1) {
		pthread_cond_wait(&(old_running->cond_var),
			&(scheduler->lock));
	}

	// old thread resumed execution
	pthread_mutex_unlock(&(scheduler->lock));


	return 0;
}

int so_signal(unsigned int io)
{
	if (io >= scheduler->io_events)
		return -1;

	int counter = 0;
	TNode *aux = NULL;

	while(scheduler->waiting_lists[io] != NULL) {
		counter++;
		aux = scheduler->waiting_lists[io];
		add_to_queue(&(scheduler->ready_queue),
			aux->element,
			cmp_by_priority);

		scheduler->waiting_lists[io] = aux->next;
		free(aux);
	}

	check_scheduler();

	return counter;
}

void so_exec(void)
{
	int n = 1 << 16;
	int i = 0;
	int a = 0;
	for (; i < n; i++) {
		a++;
	}

	check_scheduler();

	return;
}

void so_end(void)
{
	if (scheduler == NULL)
		return;

	if (scheduler->running_thread != NULL ||
		scheduler->ready_queue.size > 0)
		wait_to_finish();

	pthread_mutex_destroy(&(scheduler->fork_lock));
	pthread_mutex_destroy(&(scheduler->lock));
	pthread_barrier_destroy(&(scheduler->fork_barrier));
	pthread_cond_destroy(&(scheduler->fork_cond_var));
	pthread_cond_destroy(&(scheduler->finish_cond_var));

	int counter = 10;
	while (scheduler->thread_id_list != NULL) {
		TNode *aux = scheduler->thread_id_list;
		pthread_t t_id = *(pthread_t *)(aux->element);
		printf("cunter[%d] t_id[%ld]\n", counter, t_id);
		scheduler->thread_id_list = scheduler->thread_id_list->next;
		free(aux->element);
		free(aux);
		pthread_join(t_id, NULL);
	}

	free(scheduler);
	scheduler = NULL;
	return;
}
