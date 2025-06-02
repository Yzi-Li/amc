#include "../include/backend/object.h"
#include "../include/backend.h"
#include <stdlib.h>

extern int backend_flag;

struct object_head **objs = NULL;
int cur_obj = -1;

int object_append(struct object_head *h, struct object_node *n)
{
	if (backend_flag & BE_FLAG_STOPED)
		return 0;
	if (h == NULL || n == NULL)
		return 1;
	if (h->head == NULL) {
		h->head = n;
		h->last = n;
		n->next = NULL;
		n->prev = NULL;
		return 0;
	}
	n->prev = h->last;
	n->next = NULL;
	h->last->next = n;
	h->last = n;
	return 0;
}

int object_insert(struct object_node *src, struct object_node *n1,
		struct object_node *n2)
{
	if (backend_flag & BE_FLAG_STOPED)
		return 0;
	if (src == NULL)
		return 1;
	if (n1 != NULL)
		n1->next = src;
	src->prev = n1;
	if (n2 != NULL)
		n2->prev = src;
	src->next = n2;
	return 0;
}

int object_swap(struct object_node *n1, struct object_node *n2)
{
	if (backend_flag & BE_FLAG_STOPED)
		return 0;
	struct object_node *tmp = NULL;
	if (n1 == NULL || n2 == NULL)
		return 1;
	tmp = n1->prev;
	n1->prev = n2->prev;
	n2->prev = tmp;
	if (n1->prev != NULL)
		n1->prev->next = n1;
	if (n2->prev != NULL)
		n2->prev->next = n2;
	tmp = n1->next;
	n1->next = n2->next;
	n2->next = tmp;
	if (n1->next != NULL)
		n1->next->prev = n1;
	if (n2->next != NULL)
		n2->next->prev = n2;
	return 0;
}

void objects_free(struct object_head *o)
{
	struct object_node *cur, *nex;
	if (o->head == NULL)
		return;
	cur = o->head;
	while (cur->next != NULL) {
		nex = cur->next;
		str_free(cur->s);
		free(cur);
		cur = nex;
	}
}
