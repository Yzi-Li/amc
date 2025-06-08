#include "../include/backend/target.h"
#include "../include/parser.h"
#include "../utils/die.h"
#include <stdio.h>

static int object_write(struct object_head *obj, FILE *f);
static int objects_write(struct object_head *obj, int len, FILE *f);
static FILE *target_file_create();
static int target_err_check();

int object_write(struct object_head *obj, FILE *f)
{
	struct object_node *cur = NULL;
	if (obj == NULL)
		return 0;
	cur = obj->head;
	while (cur != NULL) {
		if (str_append(cur->s, 1, "\0"))
			return 1;
		if (fprintf(f, "%s", cur->s->s) == -1)
			return 1;
		cur = cur->next;
	}
	return 0;
}

int objects_write(struct object_head *obj, int len, FILE *f)
{
	for (int i = 0; i < len; i++) {
		if (object_write(&obj[i], f))
			return 1;
	}
	return 0;
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
	//parser_global_conf.target_name = "/tmp/amc.target.s";
	if (target_err_check())
		return 1;
	f = target_file_create();
	for (int i = 0, obj_count = cur_obj + 1;
			i < obj_count; i++) {
		if (objects_write(objs[i], len, f))
			return 1;
	}
	return fclose(f);
}
