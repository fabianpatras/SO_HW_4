#ifndef LIST_H
#define LIST_H

typedef struct node {
	void *element;
	struct node *next;
} TNode;

/* comp function:
 * returns -1 if a < b
 * 	    0 if a = b
 * 	    1 if a > b
 */
typedef int (comp_func)(void *, void *);

TNode *add_element_to_front(TNode **node, void *element);

TNode *add_element_to_sorted_list(TNode **list, void *element, comp_func cmp);

void *remove_element_from_list(TNode **list, void* element, comp_func cmp);

void *find_element(TNode *list, void *element, comp_func cmp);

#endif /* LIST_H */