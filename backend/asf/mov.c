#include "inst.h"
#include "imm.h"
#include "register.h"
#include "stack.h"
#include "suffix.h"
#include "../../utils/utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest);
static str *mov_m2r(struct asf_stack_element *src, enum ASF_REGS dest);
static str *mov_r2m(enum ASF_REGS src, struct asf_stack_element *dest);
static str *mov_r2r(enum ASF_REGS src, enum ASF_REGS dest);

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

str *mov_m2r(struct asf_stack_element *src, enum ASF_REGS dest)
{
	str *s = NULL;
	const char *temp = "mov%c -%d(%%rbp), %%%s\n";
	if (asf_regs[dest].size != src->bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 3 + ullen(src->bytes));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(src->bytes),
			src->addr,
			asf_regs[dest].name);
	return s;
}

str *mov_r2m(enum ASF_REGS src, struct asf_stack_element *dest)
{
	str *s = NULL;
	const char *temp = "mov%c %%%s, -%d(%%rbp)\n";
	if (asf_regs[src].size != dest->bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 3 + ullen(dest->bytes));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(dest->bytes),
			asf_regs[src].name,
			dest->addr);
	return s;
}

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
		return mov_m2r((struct asf_stack_element*)l,
				*(enum ASF_REGS*)r);
		break;
	case ASF_MOV_R2M:
		return mov_r2m(*(enum ASF_REGS*)l,
				(struct asf_stack_element*)r);
		break;
	case ASF_MOV_R2R:
		return mov_r2r(*(enum ASF_REGS*)l, *(enum ASF_REGS*)r);
		break;
	}

	return NULL;
}
