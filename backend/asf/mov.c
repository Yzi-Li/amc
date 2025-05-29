#include "include/mov.h"
#include "include/imm.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/suffix.h"
#include "../../utils/utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static str *mov_i2m(struct asf_imm *src, struct asf_stack_element *dest);
static str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest);
static str *mov_m2m(struct asf_stack_element *src,
		struct asf_stack_element *dest);
static str *mov_m2r(struct asf_stack_element *src, enum ASF_REGS dest);
static str *mov_m2r_converter(struct asf_stack_element *src,
		enum ASF_REGS dest);
static str *mov_m32_to_r64(struct asf_stack_element *src, enum ASF_REGS dest);
static str *mov_r2m(enum ASF_REGS src, struct asf_stack_element *dest);
static str *mov_r2r(enum ASF_REGS src, enum ASF_REGS dest);
static str *mov_u32_to_rax(str *mov);

str *mov_i2m(struct asf_imm *src, struct asf_stack_element *dest)
{
	str *s = NULL;
	const char *temp = "mov%c $%lld, -%d(%%rbp)\n";
	if (src->type != dest->bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 7
			+ ullen(src->iq)
			+ ullen(dest->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(src->type),
			src->iq,
			dest->addr);
	return s;
}

str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest)
{
	str *s = NULL;
	const char *temp = "mov%c $%lld, %%%s\n";
	if (src->type != asf_regs[dest].bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 4 + ullen(src->iq));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(asf_regs[dest].bytes),
			src->iq,
			asf_regs[dest].name);
	return s;
}

str *mov_m2m(struct asf_stack_element *src, struct asf_stack_element *dest)
{
	str *s = NULL;
	const char *temp = "mov%c -%d(%%rbp), -%d(%%rbp)\n";
	if (src->bytes != dest->bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 5
			+ ullen(src->addr)
			+ ullen(dest->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(src->bytes),
			src->addr,
			dest->addr);
	return s;
}

str *mov_m2r(struct asf_stack_element *src, enum ASF_REGS dest)
{
	str *s = NULL;
	enum ASF_BYTES src_bytes = src->bytes;
	const char *temp = "mov%c -%d(%%rbp), %%%s\n";
	if (REGION_INT(src->bytes, ASF_BYTES_U8, ASF_BYTES_U64))
		src_bytes = src->bytes - ASF_BYTES_U_OFFSET;
	if (asf_regs[dest].bytes != src_bytes)
		return mov_m2r_converter(src, dest);
	s = str_new();
	str_expand(s, strlen(temp) - 3 + ullen(src->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(src_bytes),
			src->addr,
			asf_regs[dest].name);
	return s;
}

str *mov_m2r_converter(struct asf_stack_element *src, enum ASF_REGS dest)
{
	if (asf_regs[dest].bytes == ASF_BYTES_64) {
		if (src->bytes == ASF_BYTES_32)
			return mov_m32_to_r64(src, dest);
		if (src->bytes == ASF_BYTES_U32)
			return mov_m2r(src, dest + ASF_REG_EAX);
	}
	return NULL;
}

str *mov_m32_to_r64(struct asf_stack_element *src, enum ASF_REGS dest)
{
	str *s = NULL;
	if ((s = mov_m2r(src, dest + ASF_REG_EAX)) == NULL)
		return NULL;
	return mov_u32_to_rax(s);
}

str *mov_r2m(enum ASF_REGS src, struct asf_stack_element *dest)
{
	str *s = NULL;
	const char *temp = "mov%c %%%s, -%d(%%rbp)\n";
	if (asf_regs[src].bytes != dest->bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 3 + ullen(dest->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(dest->bytes),
			asf_regs[src].name,
			dest->addr);
	return s;
}

str *mov_r2r(enum ASF_REGS src, enum ASF_REGS dest)
{
	str *s = NULL;
	const char *temp = "mov%c %%%s, %%%s\n";
	if (asf_regs[src].bytes != asf_regs[dest].bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(asf_regs[dest].bytes),
			asf_regs[src].name,
			asf_regs[dest].name);
	return s;
}

str *mov_u32_to_rax(str *mov)
{
	const char *temp = "cltq\n";
	mov->len -= 1;
	str_append(mov, strlen(temp), temp);
	return mov;
}

str *asf_inst_mov(enum ASF_MOV_TYPE mt, void *l, void *r)
{
	switch (mt) {
	case ASF_MOV_I2M:
		return mov_i2m((struct asf_imm*)l,
				(struct asf_stack_element*)r);
		break;
	case ASF_MOV_I2R:
		return mov_i2r((struct asf_imm*)l, *(enum ASF_REGS*)r);
		break;
	case ASF_MOV_M2M:
		return mov_m2m((struct asf_stack_element*)l,
				(struct asf_stack_element*)r);
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
