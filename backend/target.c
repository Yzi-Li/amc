#include "../include/backend/target.h"
#include "../include/parser.h"
#include "../utils/die.h"
#include <stdio.h>

struct object_head **objs = NULL;
int cur_obj = -1;

static int object_write(struct object_head *obj, FILE *f);
static FILE *target_file_create();
static int target_err_check();

int object_write(struct object_head *obj, FILE *f)
{
	struct object_node *cur = obj->head;
	while (cur != NULL) {
		if (str_append(cur->s, 1, "\0"))
			return 1;
		if (fprintf(f, "%s", cur->s->s) == -1)
			return 1;
		cur = cur->next;
	}
	return 0;
}

int object_append(struct object_head *h, struct object_node *n)
{
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

FILE *target_file_create()
{
	FILE *f = NULL;
	f = fopen(parser_global_conf.target_name, "w");
	if (f == NULL) {
		die ("amc: target_write: cannot write file: %s",
				parser_global_conf.target_name);
		return NULL;
	}
	return f;
}

int target_err_check()
{
	if (parser_global_conf.has_err)
		return 1;
	if (!parser_global_conf.target_name)
		return 1;
	return 0;
}

int target_write(struct object_head **objs, int len)
{
	FILE *f = NULL;
	parser_global_conf.target_name = "/tmp/amc.target.s";
	if (target_err_check())
		return 1;
	f = target_file_create();
	for (int i = 0; i < len; i++) {
		if (object_write(objs[i], f))
			return 1;
	}
	return fclose(f);
}
