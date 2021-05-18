#include <stdio.h>
#include "so_scheduler.h"
#include "list.h"

void my_func_2(unsigned int priority)
{
	printf("priority f2: [%d]\n", priority);
	so_signal(3);
	so_exec();
}

static int flag = 0;

void my_func_1(unsigned int priority)
{
	printf("priority f1: [%d]\n", priority);
	so_fork(my_func_2, 0);
	so_wait(3);
	so_exec();
}

int my_cmp(void *a, void *b)
{
	int A = *(int *)a;
	int B = *(int *)b;

	return A < B ? -1 : A == B ? 0 : 1;
}

int main() {

	so_init(3, 0);

	so_fork(my_func_1, 0);

	so_end();

	return 0;
}