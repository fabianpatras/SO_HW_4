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

	TNode *list = NULL;

	int a = 1;
	int b = 4;
	int c = 2;
	int d = 6;
	int e = 3;
	int f = 1;

	add_element_to_sorted_list(&list, &a, my_cmp);
	add_element_to_sorted_list(&list, &b, my_cmp);
	add_element_to_sorted_list(&list, &c, my_cmp);
	add_element_to_sorted_list(&list, &d, my_cmp);
	add_element_to_sorted_list(&list, &e, my_cmp);
	add_element_to_sorted_list(&list, &f, my_cmp);

	TNode *aux = list;
	while (aux != NULL) {
		printf ("element [%d] \n", *(int *)(aux->element));
		aux = aux->next;
	}
	printf("~~~~~~~\n");

	remove_element_from_list(&list, &c, my_cmp);
	remove_element_from_list(&list, &d, my_cmp);
	remove_element_from_list(&list, &a, my_cmp);
	remove_element_from_list(&list, &b, my_cmp);
	remove_element_from_list(&list, &f, my_cmp);
	remove_element_from_list(&list, &e, my_cmp);

	aux = list;

	while (aux != NULL) {
		printf ("element [%d] \n", *(int *)(aux->element));
		aux = aux->next;
	}

	return 0;
}