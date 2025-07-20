#include "../include/backend/target.h"
#include "../utils/die.h"
#include "../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__)
#include <libgen.h>
#endif

static int object_section_write(struct object_section *sec, FILE *f);
static int object_write(struct object_head *obj, FILE *f);
static FILE *target_file_create(const char *path);
static int target_file_create_dir(const char *path);

int object_section_write(struct object_section *sec, FILE *f)
{
	struct object_node *cur = NULL;
	if (sec == NULL)
		return 0;
	cur = sec->head;
	while (cur != NULL) {
		if (str_append(cur->s, 1, "\0"))
			return 1;
		if (fprintf(f, "%s", cur->s->s) == -1)
			return 1;
		cur = cur->next;
	}
	return 0;
}

int object_write(struct object_head *obj, FILE *f)
{
	for (int i = 0; i < obj->sec_count; i++) {
		if (object_section_write(&obj->sections[i], f))
			return 1;
	}
	return 0;
}

FILE *target_file_create(const char *path)
{
	FILE *f = NULL;
	if (target_file_create_dir(path))
		return NULL;
	if ((f = fopen(path, "w")) != NULL)
		return f;
	die("amc: target_write: cannot write file: %s\n", path);
	return NULL;
}

int target_file_create_dir(const char *path)
{
	str tmp = {
		.len = strlen(path),
		.s = malloc(tmp.len + 1)
	};
	char *dir = NULL;
	strncpy(tmp.s, path, tmp.len);
	dir = dirname(tmp.s);
	if (rmkdir(dir))
		goto err_free_tmp;
	free(tmp.s);
	return 0;
err_free_tmp:
	free(tmp.s);
	return 1;
}

int target_write(const char *target_path, struct object_head *obj, int len)
{
	FILE *f = NULL;
	if (target_path == NULL)
		return 1;
	if ((f = target_file_create(target_path)) == NULL)
		goto err_create_failed;
	if (object_write(cur_obj, f))
		return 1;
	return fclose(f);
err_create_failed:
	printf("amc: target_write: File: '%s' create failed!\n",
			target_path);
	return 1;
}
