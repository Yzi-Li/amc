/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/backend.h"
#include "../include/config.h"
#include "../utils/die.h"
#include <stdlib.h>
#include <string.h>

#if defined(__unix__)
#include <sys/wait.h>
#include <unistd.h>
#endif

extern struct backend backend_asf;

struct backend *backends[] = {
	NULL,
	&backend_asf
};

int backend_flag = 0;
enum BACKENDS cur_backend;

char **backend_assembled_files = NULL;
int backend_assembled_files_count = 0;
char *backend_assembler = "as";
char *backend_linker = "ld";

static int assemble_file_call_as(const char *path, int path_len, char *output);
static int link_files_call_linker(str *output);

int assemble_file_call_as(const char *path, int path_len, char *output)
{
	char *argv[] = {NULL, NULL, "-o", NULL, NULL};
	close(STDIN_FILENO);
	argv[0] = backend_assembler;
	argv[1] = malloc(path_len);
	argv[3] = output;
	strncpy(argv[1], path, path_len);
	execvp(backend_assembler, argv);
	die("amc[\x1b[30;41mDIE\x1b[0m]: Assembler stopped!\n");
	return 1;
}

int link_files_call_linker(str *output)
{
	int argv_offset = 3;
	char *argv[1 + argv_offset + backend_assembled_files_count];
	argv[0] = backend_linker;
	argv[1] = "-o";
	argv[2] = output->s;
	for (int i = 0; i < backend_assembled_files_count; i++, argv_offset++)
		argv[argv_offset] = backend_assembled_files[i];
	argv[argv_offset] = NULL;
	execvp(backend_linker, argv);
	die("amc[\x1b[30;41mDIE\x1b[0m]: Linker stopped!\n");
	return 0;
}

int backend_assemble_file(const char *path, int path_len)
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
	backend_assembled_files_count += 1;
	backend_assembled_files = realloc(backend_assembled_files,
			sizeof(*backend_assembled_files)
			* backend_assembled_files_count);
	backend_assembled_files[backend_assembled_files_count - 1] = output;
	return 0;
}

int backend_end(str *output)
{
	if (backend_flag & BE_FLAG_ENDED)
		return 0;
	return backends[cur_backend]->end(output);
}

int backend_file_end(const char *target_path, int len)
{
	if (backend_flag & BE_FLAG_STOPED)
		return 0;
	return backends[cur_backend]->file_end(target_path, len);
}

char *backend_file_get_suffix(int *result_len, int *need_free)
{
	return backends[cur_backend]->file_get_suffix(result_len, need_free);
}

int backend_file_new(struct file *f)
{
	return backends[cur_backend]->file_new(f);
}

int backend_init(int argc, char *argv[])
{
	if (backend_flag & BE_FLAG_INITED)
		return 0;
	if (cur_backend == BE_NONE)
		cur_backend = default_backend;
	return backends[cur_backend]->init(argc, argv);
}

int backend_link_files(str *output)
{
	int ret = 0;
	if ((ret = fork()) < 0)
		die("amc[\x1b[30;41mDIE\x1b[0m]: Linker fork failed!\n");
	if (ret == 0)
		return link_files_call_linker(output);
	wait(NULL);
	return 0;
}

int backend_stop(enum BE_STOP_SIGNAL bess)
{
	if (backend_flag & BE_FLAG_STOPED)
		return 0;
	if (backends[cur_backend]->stop(bess))
		return 1;
	backend_flag |= BE_FLAG_STOPED;
	return 0;
}
