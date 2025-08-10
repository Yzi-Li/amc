/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/symbol.h"
#include "include/utils.h"
#include "../include/symbol.h"
#include <stdio.h>

int symbol_read(str *result, struct file *f)
{
	if (result == NULL || f == NULL)
		return 1;
	result->s = &f->src[f->pos];
	if (isdigit(f->src[f->pos]))
		goto err_start_by_digit;
	for (; is_sym_chr(f->src[f->pos]); result->len++)
		file_pos_next(f);
	if (f->src[f->pos] != ':' && !isspace(f->src[f->pos]))
		goto err_invaild_chr;
	file_skip_space(f);
	return 0;
err_start_by_digit:
	printf("amc: symbol_read: %lld,%lld: "ERROR_STR":\n"
			"| Symbol name start by digit: '%c'\n",
			f->cur_line, f->cur_column, f->src[f->pos]);
	return 1;
err_invaild_chr:
	printf("amc: symbol_read: %lld,%lld: "ERROR_STR":\n"
			"| Invaild character: '%c'\n",
			f->cur_line, f->cur_column, f->src[f->pos]);
	return 1;
}
