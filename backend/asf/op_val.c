/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/op_val.h"

int asf_op_extract_val(struct expr *e)
{
	yz_extract_val *val = e->valr->v;
	if (e->valr->type.type == AMC_SYM) {
		return asf_op_extract_ptr_val(e->valr->v);
	} else if (e->valr->type.type == AMC_EXPR) {
		return asf_op_extract_ptr_val_from_expr(e->valr->v);
	} else if (e->valr->type.type != AMC_EXTRACT_VAL) {
		return 1;
	}
	switch (val->type) {
	case YZ_EXTRACT_ARRAY:
		return asf_op_extract_array_elem(val);
		break;
	case YZ_EXTRACT_STRUCT:
		return asf_op_extract_struct_elem(val);
		break;
	default:
		break;
		return 1;
	}
	return 0;
}
