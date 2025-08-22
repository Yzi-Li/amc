/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/checker/struct.h"
#include "../include/ptr.h"
#include <stdio.h>

int check_struct_elem_can_assign(struct symbol *ident, struct symbol *elem)
{
	yz_ptr_type *struct_ref = ident->result_type.v;
	if (!elem->flags.mut)
		goto err_elem_is_immut;
	if (ident->result_type.type != YZ_PTR) {
		if (!ident->flags.mut)
			goto err_parent_is_immut;
		return 1;
	}
	if (!struct_ref->flag_mut)
		goto err_struct_ptr_is_immut_ref;
	return 1;
err_elem_is_immut:
	printf("amc: check_struct_elem_can_assign: "ERROR_STR":\n"
			"| Assign immutable element: '%s.%s'.\n",
			ident->name.s, elem->name.s);
	return 0;
err_parent_is_immut:
	printf("amc: check_struct_elem_can_assign: "ERROR_STR":\n"
			"| Assign '%s.%s' from immutable struct.\n",
			ident->name.s, elem->name.s);
	return 0;
err_struct_ptr_is_immut_ref:
	printf("amc: check_struct_elem_can_assign: "ERROR_STR":\n"
			"| Assign '%s.%s' from pointer with "
			"immutable reference.\n",
			ident->name.s, elem->name.s);
	return 0;
}
