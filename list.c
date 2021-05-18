#include <stdlib.h>

#include "list.h"


TNode *add_element_to_front(TNode **node, void *element)
{
	TNode *new_node = calloc(1, sizeof(TNode));
	if (new_node == NULL)
		return NULL;

	new_node->element = element;
	new_node->next = *node;

	*node = new_node;
	return new_node;
}

/* adds an element to the list
 * the list should be sorted in descending order
 * when the elements are equal it skips so it can add to back
 */
TNode *add_element_to_sorted_list(TNode **list, void *element, comp_func cmp)
{
	TNode *new_node = calloc(1, sizeof(TNode));

	if (new_node == NULL)
		return NULL;

	new_node->element = element;
	new_node->next = NULL;

	// if empty list
	if (*list == NULL) {
		*list = new_node;
		return new_node;
	}

	TNode *aux, *prev;
	int inserted = 0;

	for (aux = *list, prev = NULL;
		aux != NULL;
		prev = aux, aux = aux->next) {
		if (cmp(aux->element, new_node->element) < 0) {
			/* if not first element */
			if (prev != NULL)
				prev->next = new_node;
			else
				*list = new_node;
			new_node->next = aux;
			inserted = 1;
			break;
		}
	}

	/* here the whole list was traversed but no insertion was made
	 * so we insert to back (to prev->next)
	 * prev shouldn't be NULL here
	 */
	if (inserted == 0)
		prev->next = new_node;

	return new_node;
}

void *remove_element_from_list(TNode **list, void *element, comp_func cmp)
{
	if (*list == NULL)
		return NULL;

	TNode *aux, *prev;
	void *ret = NULL;

	for (aux = *list, prev = NULL;
		aux != NULL;
		prev = aux, aux = aux->next) {

		if (cmp(aux->element, element) == 0) {
			if (prev != NULL)
				prev->next = aux->next;
			else
				*list = (*list)->next;
			ret = aux->element;
			free(aux);
			return ret;
		}
	}

	return NULL;
}

void *find_element(TNode *list, void *element, comp_func cmp)
{
	TNode *aux;

	for (aux = list; aux != NULL; aux = aux->next) {
		if (cmp(aux->element, element) == 0)
			return aux->element;
	}

	return NULL;
}

void *remove_head(TNode **list)
{
	TNode *aux = *list;

	if (aux == NULL)
		return NULL;

	void *ret = aux->element;
	*list = (*list)->next;
	free(aux);
	return ret;
}
