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

	pthread_mutex_unlock(&(scheduler->fork_lock));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~ aux_functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int is_master_thread()
{
	return scheduler->master_thread_id == pthread_self();
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
	signal_parent_thread_of_forked();

	/* TODO: some sort of lock because the thread will immediately reach
	 * this point; here we wait (somehow) to te planned on CPU
	 */


	args.handler(args.priority);

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

	
	wait_for_forked_thread(t_struct);


	/* here we wait to get signaled by the just create thread that it's
	 * ready to run before we continue;
	 * we do this with some mutex I think;
	 */



	/* this is check scheduler part */

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
