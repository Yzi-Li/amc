/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/call.h"
#include "include/stack.h"
#include "include/val.h"
#include "../../include/array.h"
#include "../../include/const.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"

static int val_get_const(yz_const *self, struct asf_val *result);
static int val_get_expr(struct expr *src, struct asf_val *result);
static int val_get_extracted_val(yz_extract_val *src,
		struct asf_val *result);
static int val_get_identifier(struct symbol *src, struct asf_val *result);
static int val_get_imm(yz_val *src, struct asf_val *result);
static int val_get_sym(struct symbol *src, struct asf_val *result);

int val_get_const(yz_const *self, struct asf_val *result)
{
	yz_array_type *arr = self->val.type.v;
	if (self->val.type.type != YZ_ARRAY)
		return 1;
	if (arr->type.type != YZ_CHAR)
		return 1;
	result->type = ASF_VAL_CONST;
	result->const_id = self->be_data.val.i;
	return 0;
}

int val_get_expr(struct expr *src, struct asf_val *result)
{
	result->type = ASF_VAL_REG;
	result->reg = asf_reg_get(asf_yz_type2bytes(src->sum_type));
	return 0;
}

int val_get_extracted_val(yz_extract_val *src, struct asf_val *result)
{
	result->type = ASF_VAL_REG;
	result->reg = asf_reg_get(asf_yz_type2bytes(&src->elem->result_type));
	return 0;
}

int val_get_identifier(struct symbol *src, struct asf_val *result)
{
	result->type = ASF_VAL_MEM;
	if (asf_stack_element2mem(src->backend_status, &result->mem) == NULL)
		goto err_identifier_not_found;
	return 0;
err_identifier_not_found:
	printf("amc[backend.asf:%s]: op_get_valr_identifier: "
			"Identifier not found: \"%s\"!\n", __FILE__,
			src->name.s);
	return 1;
}

int val_get_imm(yz_val *src, struct asf_val *result)
{
	result->type = ASF_VAL_IMM;
	if (src->type.type == YZ_NULL) {
		result->imm.type = ASF_BYTES_U64;
		result->imm.iq = 0;
		return 0;
	}
	result->imm.type = asf_yz_type2bytes(&src->type);
	result->imm.iq = src->l;
	return 0;
}

int val_get_sym(struct symbol *src, struct asf_val *result)
{
	if (src->type == SYM_IDENTIFIER)
		return val_get_identifier(src, result);
	result->type = ASF_VAL_REG;
	result->reg = asf_reg_get(asf_yz_type2bytes(&src->result_type));
	if (src->type == SYM_FUNC_ARG) {
		if (src->argc > asf_call_arg_regs_len)
			goto err_too_many_arg;
		result->reg += asf_call_arg_regs[src->argc];
	}
	return 0;
err_too_many_arg:
	printf("amc[backend.asf:%s]: val_get_sym: "
			"Unsupport number: '%d' of arguments.",
			__FILE__, src->argc);
	return 1;
}

int asf_val_get(yz_val *src, struct asf_val *result)
{
	switch (src->type.type) {
	case AMC_EXPR: return val_get_expr(src->v, result); break;
	case AMC_SYM:  return val_get_sym(src->v, result);  break;
	case AMC_EXTRACT_VAL:
		return val_get_extracted_val(src->v, result);
		break;
	case YZ_CONST: return val_get_const(src->v, result); break;
	case YZ_NULL:  return val_get_imm(src, result);      break;
	default: break;
	}
	if (YZ_IS_DIGIT(src->type.type))
		return val_get_imm(src, result);
	return 1;
}

int asf_val_is_unsigned(struct asf_val *val)
{
	switch (val->type) {
	case ASF_VAL_IMM:
		if (REGION_INT(val->imm.type, ASF_BYTES_U8, ASF_BYTES_U64))
			return 1;
		return 0;
		break;
	case ASF_VAL_MEM:
		if (REGION_INT(val->imm.type, ASF_BYTES_U8, ASF_BYTES_U64))
			return 1;
		return 0;
		break;
	case ASF_VAL_REG: return 0; break;
	default: break;
	}
	return 0;
}
