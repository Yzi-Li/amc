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
