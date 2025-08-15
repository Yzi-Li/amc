/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/mov.h"
#include "include/bytes.h"
#include "include/imm.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/suffix.h"
#include "../../utils/utils.h"
#include <stdio.h>
#include <string.h>

static str *mov_c2m(int src, struct asf_stack_element *dest);
static str *mov_c2r(int src, enum ASF_REGS dest);
static str *mov_i2m(struct asf_imm *src, struct asf_stack_element *dest);
static str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest);
static str *mov_m2m(struct asf_stack_element *src,
		struct asf_stack_element *dest);
static str *mov_m2r(struct asf_stack_element *src, enum ASF_REGS dest);
static str *mov_m2r_converter(struct asf_stack_element *src,
		enum ASF_REGS dest);
static str *mov_m32_to_r64(struct asf_stack_element *src, enum ASF_REGS dest);
static str *mov_mem_in_reg2reg(int offset, enum ASF_REGS src,
		enum ASF_REGS dest);
static str *mov_r2m(enum ASF_REGS src, struct asf_stack_element *dest);
static str *mov_r2r(enum ASF_REGS src, enum ASF_REGS dest);
static str *mov_u32_to_rax(str *mov);

str *mov_c2m(int src, struct asf_stack_element *dest)
{
	str *s = NULL;
	const char *temp = "mov%c $.LC%d, -%d(%%rbp)\n";
	s = str_new();
	str_expand(s, strlen(temp) - 5
			+ ullen(src) + ullen(dest->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(dest->bytes),
			src,
			dest->addr);
	return s;
}

str *mov_c2r(int src, enum ASF_REGS dest)
{
	str *s = NULL;
	const char *temp = "mov%c $.LC%d, %%%s\n";
	s = str_new();
	str_expand(s, strlen(temp) - 2 + ullen(src));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(asf_regs[dest].bytes),
			src,
			asf_regs[dest].name);
	return s;
}

str *mov_i2m(struct asf_imm *src, struct asf_stack_element *dest)
{
	str *s = NULL;
	enum ASF_BYTES src_bytes = asf_bytes_get_size(src->type),
	               dest_bytes = asf_bytes_get_size(dest->bytes);
	const char *temp = "mov%c $%lld, -%d(%%rbp)\n";
	if (src_bytes != dest_bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 7
			+ ullen(src->iq)
			+ ullen(dest->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(src_bytes),
			src->iq,
			dest->addr);
	return s;
}

str *mov_i2r(struct asf_imm *src, enum ASF_REGS dest)
{
	str *s = NULL;
	enum ASF_BYTES src_bytes = asf_bytes_get_size(src->type);
	const char *temp = "mov%c $%lld, %%%s\n";
	if (src_bytes != asf_regs[dest].bytes)
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
	enum ASF_BYTES src_bytes = asf_bytes_get_size(src->bytes);
	const char *temp = "mov%c -%d(%%rbp), %%%s\n";
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

str *mov_mem_in_reg2reg(int offset, enum ASF_REGS src, enum ASF_REGS dest)
{
	str *s = NULL;
	const char *temp = "mov%c (%%%s), %%%s\n";
	const char *temp_offset = "mov%c %d(%%%s), %%%s\n";
	if (asf_regs[src].bytes != ASF_BYTES_64)
		goto err_src_not_64;
	s = str_new();
	if (offset) {
		str_expand(s, strlen(temp_offset) + sllen(offset));
		snprintf(s->s, s->len, temp_offset,
				asf_suffix_get(asf_regs[dest].bytes),
				offset,
				asf_regs[src].name,
				asf_regs[dest].name);
		return s;
	}
	str_expand(s, strlen(temp) + ullen(offset));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(asf_regs[dest].bytes),
			asf_regs[src].name,
			asf_regs[dest].name);
	return s;
err_src_not_64:
	printf("amc[backend.asf]: mov_mem_in_reg2reg: "
			"src: '%s' not a 64bit register\n",
			asf_regs[src].name);
	return NULL;
}

str *mov_r2m(enum ASF_REGS src, struct asf_stack_element *dest)
{
	enum ASF_BYTES dest_bytes = asf_bytes_get_size(dest->bytes);
	str *s = NULL;
	const char *temp = "mov%c %%%s, -%d(%%rbp)\n";
	if (asf_regs[src].bytes != dest_bytes)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 3 + ullen(dest->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(dest_bytes),
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
	case ASF_MOV_C2M:
		return mov_c2m(*(int*)l, (struct asf_stack_element*)r);
		break;
	case ASF_MOV_C2R:
		return mov_c2r(*(int*)l, *(enum ASF_REGS*)r);
		break;
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

/*
 * Why fucking add this function? I don't known. But I think now I need it.
 * If I not have this function, my ass will be hurt, like f**king a sheep.
 */
str *asf_inst_mov_mem(enum ASF_MOV_MEM_TYPE type, int offset, void *l, void *r)
{
	switch (type) {
	case ASF_MOV_MEM_INREG_2_REG:
		return mov_mem_in_reg2reg(offset,
				*(enum ASF_REGS*)l, *(enum ASF_REGS*)r);
		break;
	}
	return NULL;
}
