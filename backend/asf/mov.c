#include "inst.h"
#include "imm.h"
#include "register.h"
#include "suffix.h"
#include "../../utils/utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static str *mov_r2r(enum ASF_REGS src, enum ASF_REGS dest);
static str *mov_r2r_convert(enum ASF_REGS src, enum ASF_REGS dest);
static str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest);
static str *mov_i2r_imm32_to_reg64(struct asf_imm *src,
		enum ASF_REGS dest, int sign);
static str *mov_i2r_sconvert(struct asf_imm *src, enum ASF_REGS dest);
static str *mov_i2r_uconvert(struct asf_imm *src, enum ASF_REGS dest);

str *mov_r2r(enum ASF_REGS src, enum ASF_REGS dest)
{
	const char *temp = "mov%c %%%s, %%%s\n";
	str *s = NULL;
	if (regs[src].size > regs[dest].size)
		return NULL;
	if (regs[src].size < regs[dest].size)
		return mov_r2r_convert(src, dest);
	s = str_new();
	str_expand(s, strlen(temp) - 1);
	snprintf(s->s, s->len, temp,
			suffix_get(regs[dest].size),
			regs[src].name,
			regs[dest].name);
	return s;
}

str *mov_r2r_convert(enum ASF_REGS src, enum ASF_REGS dest)
{
	const char *temp = "";
	str *s = NULL;
	return s;
}

str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest)
{
	const char *temp = "mov%c $%lld, %%%s\n";
	str *s = NULL;
	if (src->type > ASF_IMM_UNSIGNED_OFFSET) {
		if (src->type - ASF_IMM_UNSIGNED_OFFSET > regs[dest].size)
			return NULL;
		if (src->type - ASF_IMM_UNSIGNED_OFFSET == regs[dest].size)
			goto direct;
		return mov_i2r_uconvert(src, dest);
	}
	if (src->type > regs[dest].size)
		return NULL;
	if (src->type < regs[dest].size)
		return mov_i2r_sconvert(src, dest);
direct:
	s = str_new();
	str_expand(s, strlen(temp) - 4 + ullen(src->iq));
	snprintf(s->s, s->len, temp,
			suffix_get(regs[dest].size),
			src->iq,
			regs[dest].name);
	return s;
}

str *mov_i2r_imm32_to_reg64(struct asf_imm *src, enum ASF_REGS dest,
		int sign)
{
	const char *temp = "movl $%lld, %%%s\n";
	const char *temp_cdqe = "cdqe\n";
	str *s = str_new();
	str_expand(s, strlen(temp) - 4 + ullen(src->iq));
	snprintf(s->s, s->len, temp,
			src->iq,
			regs[dest + ASF_REG_32_OFFSET].name);
	s->s[s->len - 1] = '\n';
	if (sign == 1 || (sign == 2 && src->type > ASF_IMM_UNSIGNED_OFFSET))
		str_append(s, 5, temp_cdqe);
	return s;
}

str *mov_i2r_sconvert(struct asf_imm *src, enum ASF_REGS dest)
{
	const char *temp = "movs%c%c $%lld, %%%s\n";
	str *s = NULL;
	if (src->type == ASF_IMM32 && regs[dest].size == 8)
		return mov_i2r_imm32_to_reg64(src, dest, 1);
	s = str_new();
	str_expand(s, strlen(temp) - 5 + ullen(src->iq));
	snprintf(s->s, s->len, temp,
			suffix_get(src->type),
			suffix_get(regs[dest].size),
			src->iq,
			regs[dest].name);
	return s;
}

str *mov_i2r_uconvert(struct asf_imm *src, enum ASF_REGS dest)
{
	const char *temp = "movz%c%c $%lld, %%%s\n";
	str *s = NULL;
	if (src->type == ASF_IMMU32 && regs[dest].size == 8)
		return mov_i2r_imm32_to_reg64(src, dest, 0);
	str_expand(s, strlen(temp) - 5 + ullen(src->iq));
	snprintf(s->s, s->len, temp,
			suffix_get(src->type - ASF_IMM_UNSIGNED_OFFSET),
			suffix_get(regs[dest].size),
			src->iq,
			regs[dest].name);
	return s;
}

str *asf_inst_mov(enum ASF_MOV_TYPE mt, void *l, void *r)
{
	switch (mt) {
	case ASF_MOV_I2M:
		break;
	case ASF_MOV_I2R:
		return mov_i2r((struct asf_imm*)l, *(enum ASF_REGS*)r);
		break;
	case ASF_MOV_M2M:
		break;
	case ASF_MOV_M2R:
		break;
	case ASF_MOV_R2M:
		break;
	case ASF_MOV_R2R:
		return mov_r2r(*(enum ASF_REGS*)l, *(enum ASF_REGS*)r);
		break;
	}

	return NULL;
}
