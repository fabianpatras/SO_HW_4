#include "so_scheduler.h"

int so_init(unsigned int time_quantum, unsigned int io)
{
	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
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
	return;
}