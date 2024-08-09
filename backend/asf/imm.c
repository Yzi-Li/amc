#include "imm.h"
#include "../../utils/utils.h"
#include <stdio.h>

enum ASF_IMM_TYPE asf_yz_type2imm(enum YZ_TYPE type)
{
	switch (type) {
	case YZ_I8:
	case YZ_U8:
		return ASF_IMM8;
		break;
	case YZ_I16:
	case YZ_U16:
		return ASF_IMM16;
		break;
	case YZ_I32:
	case YZ_U32:
		return ASF_IMM32;
		break;
	case YZ_I64:
	case YZ_U64:
		return ASF_IMM64;
		break;
	default:
		return ASF_IMM_NULL;
		break;
	}
}

str *asf_imm_str_new(struct asf_imm *imm)
{
	str *s = str_new();
	str_expand(s, ullen(imm->iq) + 2);
	snprintf(s->s, s->len, "$%lld", imm->iq);
	return s;
}
