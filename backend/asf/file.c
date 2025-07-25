#include "include/asf.h"
#include "include/file.h"
#include "../../include/backend/target.h"
#include "../../utils/die.h"
#include <stdlib.h>
#include <string.h>

#if defined(__unix__)
#include <sys/wait.h>
#include <unistd.h>
#endif

static char *assembler = "as";
static char *linker = "ld";
static char **assembled_files = NULL;
static int assembled_files_count = 0;

static int assemble_file(const char *path, int path_len);
static int assemble_file_call_as(const char *path, int path_len, char *output);
static int link_files_call_linker(str *output);

int assemble_file(const char *path, int path_len)
{
	char *output = NULL;
	int ret = 0;
	output = malloc(path_len);
	strncpy(output, path, path_len);
	output[path_len - 2] = 'o';
	if ((ret = fork()) < 0)
		die("amc[\x1b[30;41mDIE\x1b[0m]: Assembler fork failed!\n");
	if (ret == 0)
		return assemble_file_call_as(path, path_len, output);
	wait(NULL);
	assembled_files_count += 1;
	assembled_files = realloc(assembled_files, sizeof(*assembled_files)
			* assembled_files_count);
	assembled_files[assembled_files_count - 1] = output;
	return 0;
}

int assemble_file_call_as(const char *path, int path_len, char *output)
{
	char *argv[] = {NULL, NULL, "-o", NULL, NULL};
	close(STDIN_FILENO);
	argv[0] = assembler;
	argv[1] = malloc(path_len);
	argv[3] = output;
	strncpy(argv[1], path, path_len);
	execvp(assembler, argv);
	die("amc[\x1b[30;41mDIE\x1b[0m]: Assembler stopped!\n");
	return 1;
}

int link_files_call_linker(str *output)
{
	int argv_offset = 3;
	char *argv[1 + argv_offset + assembled_files_count];
	argv[0] = linker;
	argv[1] = "-o";
	argv[2] = output->s;
	for (int i = 0; i < assembled_files_count; i++, argv_offset++)
		argv[argv_offset] = assembled_files[i];
	argv[argv_offset] = NULL;
	execvp(linker, argv);
	die("amc[\x1b[30;41mDIE\x1b[0m]: Linker stopped!\n");
	return 0;
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

int asf_link_files(str *output)
{
	int ret = 0;
	if ((ret = fork()) < 0)
		die("amc[\x1b[30;41mDIE\x1b[0m]: Linker fork failed!\n");
	if (ret == 0)
		return link_files_call_linker(output);
	wait(NULL);
	return 0;
}
