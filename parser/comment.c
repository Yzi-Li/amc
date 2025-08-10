/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/keywords.h"

int parse_comment(struct file *f)
{
	if (f->src[f->pos] == ';') {
		file_line_next(f);
		return 1;
	}
	return 0;
}
