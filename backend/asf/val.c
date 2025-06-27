#include "include/val.h"
#include "include/call.h"
#include "../../include/array.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"

static int val_get_arr(yz_array *arr, struct asf_val *result);
static int val_get_expr(struct expr *src, struct asf_val *result);
static int val_get_extracted_val(yz_extract_val *src,
		struct asf_val *result);
static int val_get_identifier(struct symbol *src, struct asf_val *result);
static int val_get_imm(yz_val *src, struct asf_val *result);
static int val_get_sym(struct symbol *src, struct asf_val *result);

int val_get_arr(yz_array *arr, struct asf_val *result)
{
	// FIXME
	if (arr->type.type != YZ_CHAR)
		return 1;
	result->type = ASF_VAL_CONST;
	result->const_id = arr->type.i;
	return 0;
}

int val_get_expr(struct expr *src, struct asf_val *result)
{
	result->type = ASF_VAL_REG;
	result->reg = asf_reg_get(asf_yz_type_raw2bytes(*src->sum_type));
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
	if ((result->mem = src->backend_status) == NULL)
		goto err_identifier_not_found;
	return 0;
err_identifier_not_found:
	printf("amc[backend.asf:%s]: op_get_valr_identifier: "
			"Identifier not found: \"%s\"!\n", __FILE__, src->name);
	return 1;
}

int val_get_imm(yz_val *src, struct asf_val *result)
{
	result->type = ASF_VAL_IMM;
	if (src->type == YZ_NULL) {
		result->imm.type = ASF_BYTES_U64;
		result->imm.iq = 0;
		return 0;
	}
	result->imm.type = asf_yz_type_raw2bytes(src->type);
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
	if (src->type == AMC_EXPR) {
		return val_get_expr(src->v, result);
	} else if (src->type == AMC_SYM) {
		return val_get_sym(src->v, result);
	} else if (src->type == AMC_EXTRACT_VAL) {
		return val_get_extracted_val(src->v, result);
	} else if (src->type == YZ_ARRAY) {
		return val_get_arr(src->v, result);
	} else if (src->type == YZ_NULL) {
		return val_get_imm(src, result);
	} else if (YZ_IS_DIGIT(src->type)) {
		return val_get_imm(src, result);
	}
	return 1;
}
