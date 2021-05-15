#include "so_scheduler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define SO_VERBOSE_ERROR

typedef struct scheduler {
	/* maximum allowed time on CPU */
	unsigned int time_quantum;

	/* maximum I/O events allowed */
	unsigned int io_events;

	/* TODO: READY queue */

	/* TODO: RUNNING identificator */

	/* */
	
} TScheduler;

static TScheduler *scheduler_instance = NULL;

int so_init(unsigned int time_quantum, unsigned int io)
{
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
	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	if (func == NULL || priority > SO_MAX_PRIO)
		return INVALID_TID;

	return INVALID_TID;
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