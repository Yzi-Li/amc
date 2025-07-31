#include "include/keywords.h"
#include "include/utils.h"
#include "../include/backend.h"
#include <stdio.h>

int err_print_pos(const char *name, const char *msg,
		i64 orig_line, i64 orig_column)
{
	if (msg != NULL) {
		printf("| %s: %lld,%lld: %s\n",
				name, orig_line, orig_column, msg);
	} else {
		printf("| %s: %lld,%lld\n", name, orig_line, orig_column);
	}
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int try_next_line(struct file *f)
{
	if (parse_comment(f))
		return 1;
	if (f->src[f->pos] != '\n')
		return 0;
	file_line_next(f);
	return 1;
}
