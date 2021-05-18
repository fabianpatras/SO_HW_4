#ifndef QUEUE_H
#define QUEUE_H

#include "list.h"

typedef struct queue {
	unsigned int size;
	TNode *container;
} TQueue;

void init_queue(TQueue *queue);
void *add_to_queue(TQueue *queue, void *element, comp_func cmp);
void *find_element_in_queue(TQueue *queue, void *element, comp_func cmp);
void *peek(TQueue *queue);
void *dequeue_pr(TQueue *queue);

#endif /* QUEUE_H */
