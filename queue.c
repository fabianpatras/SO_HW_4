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
	queue->size++;

	return element;
}

void *find_element_in_queue(TQueue *queue, void *element, comp_func cmp)
{
	return find_element(queue->container, element, cmp);
}

void *peek(TQueue *queue)
{
	if (queue->size == 0)
		return NULL;

	return queue->container->element;
}

void *dequeue_pr(TQueue *queue)
{
	if (queue->size == 0)
		exit(1);

	queue->size--;
	return remove_head(&(queue->container));
}
