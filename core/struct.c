/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/struct.h"
#include "../include/symbol.h"

void free_yz_struct(yz_struct *src)
{
	if (src == NULL)
		return;
	free_symbol_group(src->elems, src->elem_count);
	str_free_noself(&src->name);
	free(src);
}
