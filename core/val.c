/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/const.h"
#include "../include/expr.h"
#include "../include/ptr.h"
#include "../include/struct.h"
#include "../include/symbol.h"
#include "../include/val.h"
#include <stdio.h>

struct symbol *yz_get_extracted_val(yz_extract_val *val)
{
	switch (val->type) {
	case YZ_EXTRACT_STRUCT:
		return ((yz_struct*)val->sym->result_type.v)
			->elems[val->index];
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

void free_yz_extract_val(struct yz_extract_val *self)
{
	if (self == NULL)
		return;
	switch (self->type) {
	case YZ_EXTRACT_ARRAY:
		free_yz_val(self->offset);
		break;
	default: break;
	}
	free(self);
}

void free_yz_val(yz_val *self)
{
	if (self == NULL)
		return;
	free_yz_val_noself(self);
	free(self);
}

void free_yz_val_noself(yz_val *self)
{
	if (self == NULL)
		return;
	if (YZ_IS_DIGIT(self->type.type))
		return;
	free_yz_type_noself(&self->type);
	switch (self->type.type) {
	case AMC_EXPR:        free_expr(self->v);           break;
	case AMC_EXTRACT_VAL: free_yz_extract_val(self->v); break;
	case YZ_CONST:        free_yz_const(self->v);       break;
	case YZ_PTR:          free_yz_ptr(self->v);         break;
	case AMC_SYM:
	case AMC_ERR_TYPE:
	case YZ_ARRAY:
	case YZ_ENUM_ITEM:
	case YZ_NULL:
		break;
	default:
#ifdef DEBUG
		printf(">>> \x1b[34mDEBUG\x1b[0m:amc: "
				"free_yz_val_noself: "
				"Not handled type: '%s','%d'\n",
				yz_get_raw_type_name(self->type.type),
				self->type.type);
#endif
		break;
	}
}
