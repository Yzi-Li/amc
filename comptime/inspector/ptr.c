/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../../include/comptime/ptr.h"
#include "../../include/ptr.h"
#include "../../utils/utils.h"
#include <stdio.h>

int comptime_ptr_check_can_null(yz_val *val, struct symbol *sym)
{
	struct symbol *val_sym = NULL;
	yz_ptr_type *ptr;
	if (val->type.type == AMC_SYM) {
		val_sym = val->v;
		ptr = val_sym->result_type.v;
		if (!ptr->flag_can_null)
			return 1;
		if (ptr->flag_checked_null)
			return 1;
	} else if (val->type.type != YZ_NULL) {
		return 0;
	}
	if (((yz_ptr_type*)sym->result_type.v)->flag_can_null)
		return 1;
	printf("amc: comptime_ptr_check_can_null: "ERROR_STR":\n"
			"| Cannot use null for symbol: '%s'!\n"
			"| "HINT_STR": "
			"Use 'if' to check symbol before use it.\n",
			sym->name.s);
	return 0;
}

int comptime_ptr_check_can_ret(struct symbol *sym, struct symbol *fn)
{
	yz_ptr_type *ptr = sym->result_type.v;
	if (!ptr->flag_can_null)
		return 1;
	if (((yz_ptr_type*)fn->result_type.v)->flag_can_null)
		return 1;
	if (ptr->flag_checked_null)
		return 1;
	printf("amc: comptime_ptr_check_can_ret:\n"
			"| Pointer: '%s' must be checked is null.\n"
			"| Function: '%s' must return a non null pointer.\n",
			sym->name.s, fn->name.s);
	return 0;
}

int comptime_ptr_check_can_use(struct symbol *sym)
{
	yz_ptr_type *ptr = sym->result_type.v;
	if (!ptr->flag_can_null)
		return 1;
	if (ptr->flag_checked_null)
		return 1;
	printf("amc: comptime_ptr_check_can_use:\n"
			"| Pointer: '%s' must be checked is null.\n",
			sym->name.s);
	return 0;
}
