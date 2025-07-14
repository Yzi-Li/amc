#include "../include/backend/object.h"
#include "../include/backend.h"
#include <stdlib.h>

extern int backend_flag;

struct object_head *cur_obj = NULL;

int object_append(struct object_section *sec, struct object_node *n)
{
	if (backend_flag & BE_FLAG_STOPED)
		return 0;
	if (sec == NULL || n == NULL)
		return 1;
	if (sec->head == NULL) {
		sec->head = n;
		sec->last = n;
		n->next = NULL;
		n->prev = NULL;
		return 0;
	}
	n->prev = sec->last;
	n->next = NULL;
	sec->last->next = n;
	sec->last = n;
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

int object_remove(struct object_section *sec, struct object_node *n)
{
	if (sec == NULL || n == NULL)
		return 1;
	if (n == sec->last)
		return object_remove_last(sec);
	if (n->prev != NULL)
		n->prev->next = n->next;
	if (n->next != NULL)
		n->next->prev = n->prev;
	str_free(n->s);
	free(n);
	return 0;
}

int object_remove_last(struct object_section *sec)
{
	struct object_node *last = sec->last;
	if (sec == NULL)
		return 1;
	if (sec->last == NULL)
		return 1;
	if (sec->last == sec->head) {
		sec->head = NULL;
		sec->last = NULL;
	} else {
		sec->last->prev->next = NULL;
		sec->last = sec->last->prev;
	}
	str_free(last->s);
	free(last);
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

void object_head_free(struct object_head *h)
{
	if (h == NULL)
		return;
	for (int i = 0; i < h->sec_count; i++)
		object_section_free(&h->sections[i]);
	free(h);
}

void object_section_free(struct object_section *o)
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
