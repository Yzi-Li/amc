#include "../include/array.h"
#include "../include/expr.h"
#include "../include/symbol.h"
#include "../include/type.h"
#include "../include/ptr.h"
#include "../utils/utils.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

static yz_val *yz_type_arr_max(yz_val *l, yz_val *r);
static yz_val *yz_type_max_raw(enum YZ_TYPE ltype, enum YZ_TYPE rtype,
		yz_val *l, yz_val *r);
static yz_val *yz_type_ptr_max(yz_val *l, yz_val *r);

yz_val *yz_type_arr_max(yz_val *l, yz_val *r)
{
	yz_array *larr, *rarr;
	if ((larr = l->v) == NULL)
		return NULL;
	if ((rarr = r->v) == NULL)
		return NULL;
	return yz_type_max(&larr->type, &rarr->type)
		== &larr->type ? l : r;
}

yz_val *yz_type_max_raw(enum YZ_TYPE ltype, enum YZ_TYPE rtype,
		yz_val *l, yz_val *r)
{
	if (ltype == YZ_CHAR && rtype == YZ_CHAR)
		return l;
	if (!YZ_IS_DIGIT(ltype) || !YZ_IS_DIGIT(rtype))
		return NULL;
	if (YZ_IS_UNSIGNED_DIGIT(ltype)
			&& YZ_IS_UNSIGNED_DIGIT(rtype))
		return ltype > rtype ? l : r;
	if (YZ_IS_UNSIGNED_DIGIT(ltype))
		ltype = YZ_UNSIGNED_TO_SIGNED(ltype);
	if (YZ_IS_UNSIGNED_DIGIT(rtype))
		rtype = YZ_UNSIGNED_TO_SIGNED(rtype);
	return ltype > rtype ? l : r;
}

yz_val *yz_type_ptr_max(yz_val *l, yz_val *r)
{
	yz_ptr *lptr, *rptr;
	if ((lptr = yz_ptr_get_from_val(l)) == NULL)
		return NULL;
	if ((rptr = yz_ptr_get_from_val(r)) == NULL)
		return NULL;
	return yz_ptr_is_equal(lptr, rptr) ? l : NULL;
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

enum YZ_TYPE *yz_get_raw_type(yz_val *val)
{
	if (val->type == AMC_EXPR)
		return ((struct expr*)val->v)->sum_type;
	if (val->type == AMC_SYM)
		return yz_get_raw_type(
				&((struct symbol*)val->v)->result_type);
	return &val->type;
}

const char *yz_get_type_name(yz_val *val)
{
	enum YZ_TYPE type = *yz_get_raw_type(val);
	int index = type - YZ_TYPE_OFFSET;
	if (type >= YZ_TYPE_OFFSET && index < LENGTH(yz_type_table))
		return yz_type_table[index].name;
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
	case YZ_PTR:
		return yz_type_err_ptr(val);
		break;
	case YZ_ARRAY:
		return yz_type_err_array(val);
		break;
	case YZ_NULL:
		return "YZ_NULL";
		break;
	default:
		return "(Cannot get type)";
	}
	return NULL;
}

enum YZ_TYPE yz_type_get(str *s)
{
	for (int i = 0, len = LENGTH(yz_type_table); i < len; i++) {
		if (strncmp(s->s, yz_type_table[i].name, s->len) == 0)
			return yz_type_table[i].type;
	}

	return AMC_ERR_TYPE;
}

yz_val *yz_type_max(yz_val *l, yz_val *r)
{
	yz_val *result_vall = l, *result_valr = r;
	enum YZ_TYPE lraw = l->type, rraw = r->type;
	if (l->type == YZ_PTR && r->type == YZ_PTR)
		return yz_type_ptr_max(l, r);
	if ((lraw = *yz_get_raw_type(l)) == AMC_ERR_TYPE)
		return NULL;
	if ((rraw = *yz_get_raw_type(r)) == AMC_ERR_TYPE)
		return NULL;
	if ((lraw == YZ_NULL || rraw == YZ_NULL)
			&& (lraw == YZ_PTR || rraw == YZ_PTR))
		return l;
	if (lraw == YZ_PTR && rraw == YZ_PTR)
		return yz_type_ptr_max(l, r);
	if (lraw == YZ_ARRAY && rraw == YZ_ARRAY)
		return yz_type_arr_max(l, r);
	if (lraw == YZ_PTR || rraw == YZ_PTR)
		return NULL;
	if (l->type == AMC_SYM)
		result_vall = &((struct symbol*)l->v)->result_type;
	if (r->type == AMC_SYM)
		result_vall = &((struct symbol*)r->v)->result_type;
	return yz_type_max_raw(lraw, rraw, result_vall, result_valr);
}
