#include "include/asf.h"
#include "../../include/backend/target.h"
#include "../../utils/die.h"
#include <stdlib.h>
#include <string.h>

#if defined(__unix__)
#include <sys/wait.h>
#include <unistd.h>
#endif

static const char *assembler = "as";

static int assemble_file(const char *path, int path_len);
static int assemble_file_call_as(const char *path, int path_len);

int assemble_file(const char *path, int path_len)
{
	int ret = 0;
	if (ret < 0)
		die("amc[\x1b[30;41mDIE\x1b[0m]: Assembler fork failed!\n");
	if ((ret = fork()) == 0)
		return assemble_file_call_as(path, path_len);
	wait(NULL);
	return 0;
}

int assemble_file_call_as(const char *path, int path_len)
{
	char *argv[] = {NULL, NULL, "-o", NULL, NULL};
	close(STDIN_FILENO);
	argv[0] = malloc(strlen(assembler) + 1);
	argv[1] = malloc(path_len);
	argv[3] = malloc(path_len);
	strncpy(argv[0], assembler, strlen(assembler));
	strncpy(argv[1], path, path_len);
	strncpy(argv[3], path, path_len);
	argv[3][path_len - 2] = 'o';
	execvp(assembler, argv);
	die("amc[\x1b[30;41mDIE\x1b[0m]: Assembler stopped!\n");
	return 1;
}

int asf_file_end(const char *path, int path_len)
{
	struct object_head *prev = cur_obj->prev;
	if (target_write(path, path_len, cur_obj))
		return 1;
	if (assemble_file(path, path_len))
		return 1;
	object_head_free(cur_obj);
	cur_obj = prev;
	return 0;
}

char *asf_file_get_suffix(int *result_len, int *need_free)
{
	*result_len = 2;
	*need_free = 0;
	return ".s";
}

int asf_file_new(struct file *f)
{
	const char *temp_rodata = ".section .rodata\n";
	struct object_node *rodata = NULL;
	struct object_head *prev = cur_obj;
	cur_obj = calloc(1, sizeof(*cur_obj));
	cur_obj->prev = prev;
	cur_obj->sec_count = 3;
	cur_obj->sections = calloc(cur_obj->sec_count,
			sizeof(*cur_obj->sections));
	rodata = malloc(sizeof(*rodata));
	if (object_append(&cur_obj->sections[ASF_OBJ_RODATA], rodata))
		goto err_free_rodata;
	rodata->s = str_new();
	str_append(rodata->s, strlen(temp_rodata), temp_rodata);
	return 0;
err_free_rodata:
	free(rodata);
	return 1;
}
