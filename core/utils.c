#include "../include/utils.h"
#include <stdio.h>

int err_print_pos(const char *name, i64 orig_line, i64 orig_column)
{
	printf("| amc %s: %lld,%lld\n", name, orig_line, orig_column);
	return 1;
}
