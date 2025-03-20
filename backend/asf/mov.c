#include "inst.h"
#include "imm.h"
#include "register.h"
#include "suffix.h"
#include "../../utils/utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static str *mov_r2r(enum ASF_REGS src, enum ASF_REGS dest);
static str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest);

str *mov_r2r(enum ASF_REGS src, enum ASF_REGS dest)
{
	const char *temp = "mov%c %%%s, %%%s\n";
	str *s = NULL;
	if (asf_regs[src].size != asf_regs[dest].size)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(asf_regs[dest].size),
			asf_regs[src].name,
			asf_regs[dest].name);
	return s;
}

str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest)
{
	const char *temp = "mov%c $%lld, %%%s\n";
	str *s = str_new();
	str_expand(s, strlen(temp) - 4 + ullen(src->iq));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(asf_regs[dest].size),
			src->iq,
			asf_regs[dest].name);
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
