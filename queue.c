#include <stdlib.h>

#include "queue.h"

void init_queue(TQueue *queue)
{
	queue->size = 0;
	queue->container = NULL;
}

void *add_to_queue(TQueue *queue, void *element, comp_func cmp)
{
	add_element_to_sorted_list(&(queue->container), element, cmp);
}