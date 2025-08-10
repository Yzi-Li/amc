/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../../include/comptime/ptr.h"
#include "../../utils/utils.h"
#include <stdio.h>

int comptime_ptr_check_can_null(yz_val *val, struct symbol *sym)
{
	struct symbol *val_sym = NULL;
	if (val->type.type == AMC_SYM) {
		val_sym = val->v;
		if (!val_sym->flags.can_null)
			return 1;
		if (val_sym->flags.checked_null)
			return 1;
	} else if (val->type.type != YZ_NULL) {
		return 0;
	}
	if (sym->flags.can_null)
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
	if (!sym->flags.can_null)
		return 1;
	if (fn->flags.can_null)
		return 1;
	if (sym->flags.checked_null)
		return 1;
	printf("amc: comptime_ptr_check_can_ret:\n"
			"| Pointer: '%s' must be checked is null.\n"
			"| Function: '%s' must return a non null pointer.\n",
			sym->name.s, fn->name.s);
	return 0;
}

int comptime_ptr_check_can_use(struct symbol *sym)
{
	if (!sym->flags.can_null)
		return 1;
	if (sym->flags.checked_null)
		return 1;
	printf("amc: comptime_ptr_check_can_use:\n"
			"| Pointer: '%s' must be checked is null.\n",
			sym->name.s);
	return 0;
}
