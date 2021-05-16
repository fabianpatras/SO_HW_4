#include "so_scheduler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define SO_VERBOSE_ERROR

typedef struct pthread_arg {
	so_handler *handler;
	unsigned int priority;
	int is_master;
} TPthread_arg;

typedef struct thread_struct {
	tid_t id;
	/* mutex ceva? */

} TThread_struct;

typedef struct scheduler {
	/* maximum allowed time on CPU */
	unsigned int max_time_quantum;

	/* maximum I/O events allowed */
	unsigned int io_events;

	/* the id of the currently running thread */
	tid_t master_thread_id;

	/* the id of the currently running thread */
	tid_t crt_running_thread_id;

	/* current thread time on CPU */
	unsigned int crt_thread_time_quantum;

	/* lock used to operate on scheduler internals */
	pthread_mutex_t scheduler_lock;

	/* TODO: READY queue */

	/* TODO: RUNNING identificator */

	/* ??? */
	
} TScheduler;

static TScheduler *scheduler_instance = NULL;

int so_init(unsigned int time_quantum, unsigned int io)
{
	int rc = 0;

	// printf("[%p]\n", scheduler_instance);
	if (scheduler_instance != NULL) {
		// printf("already initialised");
		return -1;
	}

	if (io > SO_MAX_NUM_EVENTS) {
		return -1;
	}

	if (time_quantum == 0)
		return -1;

	scheduler_instance = calloc(1, sizeof(TScheduler));
	if (scheduler_instance == NULL)
		return -1;
	
	// printf("init ok [%p]\n", scheduler_instance);
	scheduler_instance->io_events = io;
	scheduler_instance->max_time_quantum = time_quantum;
	scheduler_instance->crt_thread_time_quantum = 0;
	scheduler_instance->crt_running_thread_id = INVALID_TID;
	rc = pthread_mutex_init(&(scheduler_instance->scheduler_lock), NULL);

	if (rc != 0)
		goto err_free;

	scheduler_instance->master_thread_id = pthread_self();

	return 0;

err_free:
	free(scheduler_instance);
	scheduler_instance = NULL;
	return -1;
}

int is_master_thread()
{
	return scheduler_instance->master_thread_id == pthread_self();
}

static void *start_function(void *arg)
{
	TPthread_arg args = *(TPthread_arg *)arg;
	free(arg);

	/* TODO: some sort of lock because the thread will immediately reach
	 * this point
	 */
	

	args.handler(args.priority);

	pthread_exit(NULL);
}

/* asta e o instructiune deci consuma o cunata de timp*/
tid_t so_fork(so_handler *func, unsigned int priority)
{
	if (func == NULL || priority > SO_MAX_PRIO)
		return INVALID_TID;

	TPthread_arg *arg = calloc(1, sizeof(TPthread_arg));
	if (arg == NULL)
		return INVALID_TID;
	
	arg->handler = func;
	arg->priority = priority;
	arg->is_master = is_master_thread();

	pthread_t new_thread = INVALID_TID;
	pthread_create(&new_thread, NULL, start_function, arg);

	return new_thread;
}

/* asta e o instructiune deci consuma o cunata de timp*/
int so_wait(unsigned int io)
{
	// asta e o instuctiune
	return -1;
}

/* asta e o instructiune deci consuma o cunata de timp*/
int so_signal(unsigned int io)
{
	return -1;
}

/* asta e o instructiune deci consuma o cunata de timp*/
void so_exec(void)
{
	// aici se cosidera ca e executata instructiunea
	// adica la un apel de so_exec se considera cu threadul curent a
	// stat o cuanta de timp pe procesor

	// pthread_t self_id = pthread_self();
	// contorizez ca thread[self_id] a mai executat o coanta de timp

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
	if (scheduler_instance != NULL) {
		free(scheduler_instance);
		scheduler_instance = NULL;
		// printf("after free [%p]\n", scheduler_instance);
	}
	// printf("so_end ok [%p]\n", scheduler_instance);
	return;
}