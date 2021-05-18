#include <stdio.h>
#include "so_scheduler.h"
#include "list.h"

void my_printf2(unsigned int priority)
{
	printf("priority f2: [%d]\n", priority);
}

static int flag = 0;

void my_printf(unsigned int priority)
{
	if (flag == 0) {
		flag = 1;
		so_fork(my_printf, priority);
		printf("\t\t\t\t\t\t(my_printf)(so_exec_0)(%ld)\n", pthread_self());
	}

	printf("priority: [%d]\n", priority);
	so_exec();
	printf("\t\t\t\t\t\t(my_printf)(so_exec_1)(%ld)\n", pthread_self());
	so_exec();
	printf("\t\t\t\t\t\t(my_printf)(so_exec_2)(%ld)\n", pthread_self());
	so_exec();
	printf("\t\t\t\t\t\t(my_printf)(so_exec_3)(%ld)\n", pthread_self());
	so_exec();
	printf("\t\t\t\t\t\t(my_printf)(so_exec_4)(%ld)\n", pthread_self());
	so_exec();
	printf("\t\t\t\t\t\t(my_printf)(so_exec_5)(%ld)\n", pthread_self());
	// so_fork(my_printf2, 3);
	// so_fork(my_printf2, 4);
	// printf("(my_printf)(so_exec_1)\n");
	// so_exec();
	// printf("(my_printf)(so_exec_2)\n");
	// so_exec();
	// printf("(my_printf)(so_exec_3)\n");
}

int my_cmp(void *a, void *b)
{
	int A = *(int *)a;
	int B = *(int *)b;

	return A < B ? -1 : A == B ? 0 : 1;
}

int main() {

	so_init(2, 0);

	so_fork(my_printf, 0);

	so_end();

	return 0;
}