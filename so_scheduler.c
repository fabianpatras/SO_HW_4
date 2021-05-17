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
	/* thread id */
	tid_t id;

	/* the condition variable that will control the thread
	 * it does so when we call pthread_cond_wait and pthread_cond_signal
	 * on this varaible and the scheduler_instance
	 * TODO: mutex??
	 */
	pthread_cond_t cond_var;

	/* variable used to check if we can run or not
	 */
	unsigned int condition;

	/* how much CPU time this thread had without being preempted
	 * this should reset to 0 when barely planned for execution
	 */
	unsigned int quantum_spent;
} TThread_struct;

typedef struct scheduler {
	/* maximum allowed time on CPU */
	unsigned int max_time_quantum;

	/* maximum I/O events allowed */
	unsigned int io_events;

	/* the id of the master thread == thread that called so_init */
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


static int is_master_thread()
{
	return scheduler_instance->master_thread_id == pthread_self();
}

static void scheduler_lock()
{
	pthread_mutex_lock(&(scheduler_instance->scheduler_lock));
}

static void scheduler_unlock()
{
	pthread_mutex_unlock(&(scheduler_instance->scheduler_lock));
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
	 */


	args.handler(args.priority);

	pthread_exit(NULL);
}

int so_init(unsigned int time_quantum, unsigned int io)
{
	int rc = 0;

	if (scheduler_instance != NULL) {
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
	pthread_join(new_thread, NULL);

	/* here we wait to get singaled by the just create thread that it's
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
	if (scheduler_instance != NULL) {
		free(scheduler_instance);
		scheduler_instance = NULL;
	}

	return;
}
