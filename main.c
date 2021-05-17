#include <stdio.h>
#include "so_scheduler.h"

void my_printf(unsigned int priority)
{
	printf("priority: [%d]\n", priority);
}

int main() {

	so_init(2, 0);

	printf("salut lume!\n");

	so_fork(my_printf, 3);

	so_end();
	return 0;
}