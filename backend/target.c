#include "../include/backend/target.h"
#include "../utils/die.h"
#include <stdio.h>

static int object_write(struct object_head *obj, FILE *f);
static int objects_write(struct object_head *obj, int len, FILE *f);
static FILE *target_file_create(const char *path);

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

FILE *target_file_create(const char *path)
{
	FILE *f = NULL;
	f = fopen(path, "w");
	if (f != NULL)
		return f;
	die("amc: target_write: cannot write file: %s", path);
	return NULL;
}

int target_write(const char *target_path, struct object_head *obj, int len)
{
	FILE *f = NULL;
	if (target_path == NULL)
		return 1;
	f = target_file_create(target_path);
	if (objects_write(cur_obj, len, f))
		return 1;
	return fclose(f);
}
