#ifndef THREAD_UTIL_H
#define THREAD_UTIL_H

#include <pthread.h>
#include "queue.h"

typedef struct pthread_arg {
	so_handler *handler;
	unsigned int priority;
	int is_master;
} TPthread_arg;

typedef struct thread_struct {
	/* thread id */
	pthread_t id;

	/* thread static priority */
	unsigned int priority;

	/* the condition variable that will control the thread
	 * it does so when we call pthread_cond_wait and pthread_cond_signal
	 * on this varaible and the scheduler_instance
	 * TODO: mutex??
	 */
	pthread_cond_t cond_var;

	/* variable used to check if we can run or not
	 */
	volatile unsigned int condition;

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
	pthread_t master_thread_id;

	/* the id of the currently running thread */
	pthread_t crt_running_thread_id;

	/* current thread time on CPU */
	unsigned int crt_thread_time_quantum;

	/* lock used to operate on scheduler internals */
	pthread_mutex_t lock;

	/* lock used to operate on fork_add_to_ready_queue */
	pthread_mutex_t fork_lock;

	/* cond var used fro fork_add_to_ready_queue */
	pthread_cond_t fork_cond_var;

	/* barrier used to catch the newly spawned thread */
	pthread_barrier_t fork_barrier;

	/* cond used fro fork_add_to_ready_queue */
	unsigned int fork_flag;

	/* cond var used for signaling all threads finished executing*/
	pthread_cond_t finish_cond_var;

	/* cond used for checking if all threads finished executing */
	unsigned int finish_flag;

	/* TODO: READY queue */
	TQueue ready_queue;

	/* TODO: RUNNING identificator */
	TThread_struct *running_thread;

	/* a list with all the thread id's that have to be joined at the end */
	TNode *thread_id_list;

	/* ??? */

} TScheduler;

static int cmp_by_priority(void *a, void *b)
{
	TThread_struct t1 = *(TThread_struct *)a;
	TThread_struct t2 = *(TThread_struct *)b;

	return t1.priority < t2.priority ? -1 : 
		t1.priority == t2.priority ? 0 : 1;
}

static int cmp_by_priority_2(void *a, void *b)
{
	TThread_struct t1 = *(TThread_struct *)a;
	TThread_struct t2 = *(TThread_struct *)b;

	return t1.priority > t2.priority ? -1 : 
		t1.priority == t2.priority ? -1 : 1;
}

/* a - TThread_struct *
 * b - pthread_t *
 */
static int cmp_find_by_id(void *a, void *b)
{
	TThread_struct t1 = *(TThread_struct *)a;
	pthread_t id = *(pthread_t *)b;

	// printf("id[%ld]id[%ld]equal[%d]\n", t1.id, id,
	// 	pthread_equal(t1.id, id));

	return pthread_equal(t1.id, id) == 0 ? -1 : 0;
}

#endif /* THREAD_UTIL_H */