/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/checker/ptr.h"
#include "../include/checker/symbol.h"
#include <stdio.h>

int check_sym_can_assign(struct symbol *sym)
{
	if (!sym->flags.mut && sym->flags.is_init)
		goto err_sym_is_immut;
	return 1;
err_sym_is_immut:
	printf("amc: check_sym_can_assign: "
			"ERROR: Symbol: \"%s\" is immutable!\n", sym->name.s);
	return 0;
}

int check_sym_can_assign_val(struct symbol *sym, yz_val *val)
{
	if (sym->result_type.type != YZ_PTR && val->type.type == YZ_NULL) {
		if (!check_ptr_can_null(val, sym))
			return 0;
		return 1;
	}
	return 1;
}
