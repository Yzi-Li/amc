/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/array.h"
#include "../include/enum.h"
#include "../include/expr.h"
#include "../include/struct.h"
#include "../include/symbol.h"
#include "../include/type.h"
#include "../include/ptr.h"
#include "../utils/utils.h"
#include <limits.h>
#include <string.h>

static yz_type *yz_type_max_digit(enum YZ_TYPE ltype, enum YZ_TYPE rtype,
		yz_type *l, yz_type *r);
static yz_type *yz_type_max_raw(yz_type *lraw, yz_type *rraw,
		yz_type *l, yz_type *r);

yz_type *yz_type_max_digit(enum YZ_TYPE ltype, enum YZ_TYPE rtype,
		yz_type *l, yz_type *r)
{
	if (!YZ_IS_DIGIT(ltype) || !YZ_IS_DIGIT(rtype))
		return NULL;
	if (YZ_IS_UNSIGNED_DIGIT(ltype) && YZ_IS_UNSIGNED_DIGIT(rtype))
		return ltype > rtype ? l : r;
	if (YZ_IS_UNSIGNED_DIGIT(ltype))
		ltype = YZ_UNSIGNED_TO_SIGNED(ltype);
	if (YZ_IS_UNSIGNED_DIGIT(rtype))
		rtype = YZ_UNSIGNED_TO_SIGNED(rtype);
	return ltype > rtype ? l : r;
}

yz_type *yz_type_max_raw(yz_type *lraw, yz_type *rraw, yz_type *l, yz_type *r)
{
	if (lraw->type == YZ_CHAR && rraw->type == YZ_CHAR)
		return l;
	if (lraw->type == YZ_PTR && rraw->type == YZ_PTR)
		return yz_type_max_ptr(lraw, rraw, l, r);
	if (lraw->type == YZ_PTR && rraw->type == YZ_NULL)
		return l;
	if (lraw->type == YZ_NULL && rraw->type == YZ_PTR)
		return r;
	if (lraw->type == YZ_ARRAY && rraw->type == YZ_ARRAY)
		return yz_type_max_arr(l, r);
	if (YZ_IS_DIGIT(lraw->type) && YZ_IS_DIGIT(rraw->type))
		return yz_type_max_digit(lraw->type, rraw->type, l, r);
	return NULL;
}

enum YZ_TYPE yz_get_int_size(long long l)
{
	if (l <= CHAR_MAX)
		return YZ_I8;
	if (l <= SHRT_MAX)
		return YZ_I16;
	if (l <= INT_MAX)
		return YZ_I32;
	if (l <= LLONG_MAX)
		return YZ_I64;
	return AMC_ERR_TYPE;
}

yz_type *yz_get_raw_type(yz_type *type)
{
	switch (type->type) {
	case AMC_EXPR:
		return yz_get_raw_type(((struct expr*)type->v)->sum_type);
		break;
	case AMC_SYM:
		return yz_get_raw_type(
				&((struct symbol*)type->v)->result_type);
		break;
	case AMC_EXTRACT_VAL:
		return yz_get_raw_type(&yz_get_extracted_val(type->v)
				->result_type);
		break;
	case YZ_CONST:
		return yz_get_raw_type(type->v);
		break;
	default: break;
	}
	return type;
}

const char *yz_get_raw_type_name(enum YZ_TYPE type)
{
	switch (type) {
	case AMC_ERR_TYPE:
		return "AMC_ERR_TYPE";
		break;
	case AMC_SYM:
		return "AMC_SYM";
		break;
	case AMC_EXPR:
		return "AMC_EXPR";
		break;
	case AMC_EXTRACT_VAL:
		return "AMC_EXTRACTED_VAL";
		break;
	case YZ_ARRAY:
		return "YZ_ARRAY";
		break;
	case YZ_CONST:
		return "YZ_CONST";
		break;
	case YZ_NULL:
		return "YZ_NULL";
		break;
	case YZ_PTR:
		return "YZ_PTR";
		break;
	case YZ_STRUCT:
		return "YZ_STRUCT";
		break;
	default:
		return "(Cannot get type)";
		break;
	}
}

const char *yz_get_type_name(yz_type *type)
{
	yz_type *raw = yz_get_raw_type(type);
	int index = raw->type - YZ_TYPE_OFFSET;
	if (raw->type >= YZ_TYPE_OFFSET && index < LENGTH(yz_type_table))
		return yz_type_table[index].name;
	return yz_get_raw_type_name(raw->type);
}

enum YZ_TYPE yz_type_get(str *s)
{
	for (int i = 0, len = LENGTH(yz_type_table); i < len; i++) {
		if (strncmp(s->s, yz_type_table[i].name, s->len) == 0)
			return yz_type_table[i].type;
	}
	return AMC_ERR_TYPE;
}

yz_type *yz_type_max(yz_type *l, yz_type *r)
{
	yz_type *lraw = yz_get_raw_type(l), *rraw = yz_get_raw_type(r);
	if (lraw->type == AMC_ERR_TYPE || rraw->type == AMC_ERR_TYPE)
		return NULL;
	return yz_type_max_raw(lraw, rraw, l, r);
}

void free_yz_type(yz_type *self)
{
	if (self == NULL)
		return;
	free_yz_type_noself(self);
	free(self);
}

void free_yz_type_noself(yz_type *self)
{
	if (self == NULL || self->v == NULL)
		return;
	self->v = NULL;
	if (YZ_IS_DIGIT(self->type))
		return;
	switch (self->type) {
	case AMC_EXPR:
		free_expr(self->v);
		break;
	case AMC_EXTRACT_VAL:
		free_yz_extract_val(self->v);
		break;
	case YZ_PTR:
		free_yz_ptr_type(self->v);
		return;
		break;
	default:
#ifdef DEBUG
		printf(">>> \x1b[34mDEBUG\x1b[0m:amc: "
				"free_yz_type_noself: "
				"Not handled type: '%s'\n",
				yz_get_raw_type_name(self->type));
#endif
		return;
		break;
	}
}

void free_yz_user_type(void *self)
{
	yz_user_type *type = self;
	if (self == NULL)
		return;
	switch (type->type) {
	case YZ_ENUM:   free_yz_enum(type->enum_);     break;
	case YZ_STRUCT: free_yz_struct(type->struct_); break;
	default: break;
	}
	free(self);
}
