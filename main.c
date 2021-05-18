#include <stdio.h>
#include "so_scheduler.h"
#include "list.h"

void my_printf(unsigned int priority)
{
	printf("priority: [%d]\n", priority);
}

int my_cmp(void *a, void *b)
{
	int A = *(int *)a;
	int B = *(int *)b;

	return A < B ? -1 : A == B ? 0 : 1;
}

int main() {

	so_init(2, 0);

	so_fork(my_printf, 2);

	so_end();

	return 0;
}