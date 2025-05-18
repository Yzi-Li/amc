#include "include/imm.h"
#include "../../include/expr.h"
#include "../../utils/utils.h"
#include <stdio.h>

enum ASF_IMM_TYPE asf_yz_type_raw2imm(enum YZ_TYPE type)
{
	switch (type) {
	case YZ_I8:
	case YZ_I16:
	case YZ_I32:
		return ASF_IMM32;
		break;
	case YZ_U8:
	case YZ_U16:
	case YZ_U32:
		return ASF_IMMU32;
		break;
	case YZ_I64: return ASF_IMM64; break;
	case YZ_PTR:
	case YZ_U64:
		return ASF_IMMU64;
		break;
	default: return ASF_IMM_NULL; break;
	}
	return ASF_IMM_NULL;
}

enum ASF_IMM_TYPE asf_yz_type2imm(yz_val *type)
{
	if (type == NULL)
		return ASF_IMM_NULL;
	switch (type->type) {
	case AMC_EXPR:
		return asf_yz_type_raw2imm(
				*((struct expr*)type->v)->sum_type);
		break;
	case YZ_PTR:
		return ASF_IMM_PTR;
		break;
	default:
		return asf_yz_type_raw2imm(type->type);
		break;
	}
	return ASF_IMM_NULL;
}

str *asf_imm_str_new(struct asf_imm *imm)
{
	str *s = str_new();
	str_expand(s, ullen(imm->iq) + 2);
	snprintf(s->s, s->len, "$%lld", imm->iq);
	return s;
}
