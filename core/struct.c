/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/struct.h"
#include "../include/symbol.h"
#include "../utils/utils.h"

void free_yz_struct(yz_struct *src)
{
	free_symbol_group(src->elems, src->elem_count);
	str_free(&src->name);
	free_safe(src);
}
