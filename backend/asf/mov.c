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

static str *mov_m2r_converter(struct asf_mem *src,
		enum ASF_REGS dest);
static str *mov_m32_to_r64(struct asf_mem *src, enum ASF_REGS dest);
static str *mov_u32_to_rax(str *mov);

str *mov_m2r_converter(struct asf_mem *src, enum ASF_REGS dest)
{
	if (asf_regs[dest].bytes == ASF_BYTES_64) {
		if (src->bytes == ASF_BYTES_32)
			return mov_m32_to_r64(src, dest);
		if (src->bytes == ASF_BYTES_U32)
			return asf_inst_mov_m2r(src, dest + ASF_REG_EAX);
	}
	return NULL;
}

str *mov_m32_to_r64(struct asf_mem *src, enum ASF_REGS dest)
{
	str *s = NULL;
	if ((s = asf_inst_mov_m2r(src, dest + ASF_REG_EAX)) == NULL)
		return NULL;
	return mov_u32_to_rax(s);
}

str *mov_u32_to_rax(str *mov)
{
	const char *temp = "cltq\n";
	mov->len -= 1;
	str_append(mov, strlen(temp), temp);
	return mov;
}

str *asf_inst_mov_c2m(int src, struct asf_mem *dest)
{
	str *s = NULL;
	const char *temp = "mov%c $.LC%d, (%%%s)\n";
	const char *temp_offset = "mov%c $.LC%d, %d(%%%s)\n";
	s = str_new();
	if (dest->addr) {
		str_expand(s, strlen(temp_offset) - 4
				+ ullen(src)
				+ sllen(dest->offset));
		snprintf(s->s, s->len, temp_offset,
				asf_suffix_get(dest->bytes),
				src,
				dest->offset,
				asf_regs[dest->addr].name);
		return s;
	}
	str_expand(s, strlen(temp) - 2 + ullen(src));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(dest->bytes),
			src,
			asf_regs[dest->addr].name);
	return s;
}

str *asf_inst_mov_c2r(int src, enum ASF_REGS dest)
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

str *asf_inst_mov_i2m(struct asf_imm *src, struct asf_mem *dest)
{
	str *s = NULL;
	enum ASF_BYTES src_bytes = asf_bytes_get_size(src->type),
	               dest_bytes = asf_bytes_get_size(dest->bytes);
	const char *temp = "mov%c $%lld, (%%%s)\n";
	const char *temp_offset = "mov%c $%lld, %d(%%%s)\n";
	if (src_bytes != dest_bytes)
		return NULL;
	s = str_new();
	if (dest->offset) {
		str_expand(s, strlen(temp_offset) - 6
				+ ullen(src->iq)
				+ sllen(dest->offset));
		snprintf(s->s, s->len, temp_offset,
				asf_suffix_get(src_bytes),
				src->iq,
				dest->offset,
				asf_regs[dest->addr].name);
		return s;
	}
	str_expand(s, strlen(temp) - 4 + ullen(src->iq));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(src_bytes),
			src->iq,
			asf_regs[dest->addr].name);
	return s;
}

str *asf_inst_mov_i2r(struct asf_imm *src, enum ASF_REGS dest)
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

str *asf_inst_mov_m2m(struct asf_mem *src, struct asf_mem *dest)
{
	int reg_pushed = 0;
	str *result, *tmp_str;
	enum ASF_REGS tmp_reg = ASF_REG_RDX;
	if (*asf_regs[tmp_reg].purpose != ASF_REG_PURPOSE_NULL) {
		reg_pushed = 1;
		asf_inst_push_reg(tmp_reg);
	}
	tmp_reg += asf_reg_get(src->bytes);
	if ((result = asf_inst_mov_m2r(src, tmp_reg)) == NULL)
		return NULL;
	if ((tmp_str = asf_inst_mov_r2m(tmp_reg, dest)) == NULL)
		goto err_free_result;
	str_append(result, tmp_str->len, tmp_str->s);
	str_free(tmp_str);
	if (reg_pushed) {
		if ((tmp_str = asf_inst_pop(tmp_reg)) == NULL)
			goto err_free_result;
		str_append(result, tmp_str->len, tmp_str->s);
		str_free(tmp_str);
	}
	return result;
err_free_result:
	str_free(result);
	return NULL;
}

str *asf_inst_mov_m2r(struct asf_mem *src, enum ASF_REGS dest)
{
	str *s = NULL;
	enum ASF_BYTES src_bytes = asf_bytes_get_size(src->bytes);
	const char *temp = "mov%c (%%%s), %%%s\n";
	const char *temp_offset = "mov%c %d(%%%s), %%%s\n";
	if (asf_regs[dest].bytes != src_bytes)
		return mov_m2r_converter(src, dest);
	s = str_new();
	if (src->offset) {
		str_expand(s, strlen(temp_offset) - 2 + sllen(src->offset));
		snprintf(s->s, s->len, temp_offset,
				asf_suffix_get(src_bytes),
				src->offset,
				asf_regs[src->addr].name,
				asf_regs[dest].name);
		return s;
	}
	str_expand(s, strlen(temp));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(src_bytes),
			asf_regs[src->addr].name,
			asf_regs[dest].name);
	return s;
}

str *asf_inst_mov_r2m(enum ASF_REGS src, struct asf_mem *dest)
{
	enum ASF_BYTES dest_bytes = asf_bytes_get_size(dest->bytes);
	str *s = NULL;
	const char *temp = "mov%c %%%s, (%%%s)\n";
	const char *temp_offset = "mov%c %%%s, %d(%%%s)\n";
	if (asf_regs[src].bytes != dest_bytes)
		return NULL;
	s = str_new();
	if (dest->offset) {
		str_expand(s, strlen(temp_offset) - 2 + sllen(dest->offset));
		snprintf(s->s, s->len, temp_offset,
				asf_suffix_get(dest_bytes),
				asf_regs[src].name,
				dest->offset,
				asf_regs[dest->addr].name);
		return s;
	}
	str_expand(s, strlen(temp));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(dest_bytes),
			asf_regs[src].name,
			asf_regs[dest->addr].name);
	return s;
}

str *asf_inst_mov_r2r(enum ASF_REGS src, enum ASF_REGS dest)
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
