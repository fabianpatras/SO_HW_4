#include "so_scheduler.h"
#include "thread_util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define SO_VERBOSE_ERROR

static TScheduler *scheduler = NULL;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ thread_sync ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void thread_wait_to_get_scheduled(TThread_struct *thread)
{
	pthread_mutex_lock(&(scheduler->lock));

	while (!(thread->condition)) {
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

static void wait_for_forked_thread(TThread_struct *thread)
{
	pthread_mutex_lock(&(scheduler->fork_lock));

	while(!(scheduler->fork_flag)) {
		pthread_cond_wait(&(scheduler->fork_cond_var),
			&(scheduler->fork_lock));
	}

	add_to_queue(&(scheduler->ready_queue), thread, cmp_by_priority);

	printf("threadul [%ld] cu priority [%d] a fost bagat in coada\n",
		thread->id, thread->priority);
	
	scheduler->fork_flag = 0;

	pthread_mutex_unlock(&(scheduler->fork_lock));
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
	TThread_struct *crt_thread = get_thread_struct_from_ready();

	pthread_mutex_lock(&(scheduler->lock));
	pthread_cond_wait(&(crt_thread->cond_var), &(scheduler->lock));
	pthread_mutex_unlock(&(scheduler->lock));
}

static void check_scheduler()
{
	TThread_struct *crt_thread = get_thread_struct_from_ready();

	if (crt_thread == NULL &&
		pthread_equal(pthread_self(), scheduler->master_thread_id)) {
		// here we should somehow start the scheduling 
		// but not block here
		TThread_struct *first_thread = get_first_element(&(scheduler->ready_queue));
		scheduler->crt_running_thread_id = first_thread->id;
		
		// this instruction starts the first forked thread
		printf("Starting the first forked thread\n");
		pthread_cond_signal(&(first_thread->cond_var));
		return;
	}

	pthread_mutex_lock(&(scheduler->lock));
	scheduler->crt_thread_time_quantum++;
	if (scheduler->crt_thread_time_quantum == scheduler->max_time_quantum) {
		// TODO: 1) maine dim trebuie mutat din a tine id-ul treadurlui
		// care ruleaza in a tine un pointer la structura threadului
		// care ruleaza
		//
		// 2) dupa asta treburile ar trebuis a devina mai usoare pentru
		// ca acum putem sa punem threadurile pe sleep atunci cand sunt
		// spawnate si sa le dam signal
		//
		// 3) aici cand trebuie sa facem switch la threaduri pentru ca
		// au rulat prea mult ele trebuie adaugate in priority queue
		// in coada la ac prioritate si threadul care e pe prima pozitie
		// in coada de ready trebuie scos si pus in running
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
	// signal_parent_thread_of_forked();

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
	scheduler->fork_flag = 0;
	pthread_barrier_init(&(scheduler->fork_barrier), NULL, 2);

	scheduler->master_thread_id = pthread_self();
	init_queue(&(scheduler->ready_queue));

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

	/* here we wait to get signaled by the just create thread that it's
	 * ready to run before we continue;
	 * we do this with some mutex I think;
	 */
	add_to_queue(&(scheduler->ready_queue), t_struct, cmp_by_priority);
	// wait_for_forked_thread(t_struct);

	/* this is check scheduler part */
	check_scheduler();
	

	return new_thread;
}

int so_wait(unsigned int io)
{
	return -1;
}

int so_signal(unsigned int io)
{
	return -1;
}

void so_exec(void)
{
	int n = 1 << 12;
	int i = 0;
	int a = 0;
	for (; i < n; i++) {
		a++;
	}
	return;
}

void so_end(void)
{
	if (scheduler != NULL) {
		free(scheduler);
		scheduler = NULL;
	}

	return;
}
