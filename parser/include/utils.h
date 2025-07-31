#ifndef AMC_PARSER_UTILS_H
#define AMC_PARSER_UTILS_H
#include "../../include/file.h"
#include "../../utils/cint.h"

#define ERROR_STR "\x1b[31merror\x1b[0m"

int err_print_pos(const char *name, const char *msg,
		i64 orig_line, i64 orig_column);
int try_next_line(struct file *f);

#endif
