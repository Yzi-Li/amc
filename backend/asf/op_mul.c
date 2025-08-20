/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/suffix.h"
#include <stdlib.h>
#include <string.h>

static str *mul_imm_to_reg(struct asf_imm *src, int is_unsigned);
static str *mul_mem_or_reg_to_reg(struct asf_val *src, int is_unsigned);

str *mul_imm_to_reg(struct asf_imm *src, int is_unsigned)
{
	str *s = NULL, *tmp = NULL;
	struct asf_val src_wrap = {.type = ASF_VAL_REG, .reg = ASF_REG_RBX};
	src_wrap.reg += asf_reg_get(src->type);
	if ((s = asf_inst_mov_i2r(src, src_wrap.reg)) == NULL)
		return NULL;
	if ((tmp = mul_mem_or_reg_to_reg(&src_wrap, is_unsigned)) == NULL)
		goto err_free_s;
	str_append(s, tmp->len, tmp->s);
	str_free(tmp);
	return s;
err_free_s:
	str_free(s);
	return NULL;
}

str *mul_mem_or_reg_to_reg(struct asf_val *src, int is_unsigned)
{
	enum ASF_BYTES bytes;
	str *s = NULL, *tmp = NULL;
	const char *temp_signed = "imul%c %s\n",
	           *temp_unsigned = "mul%c %s\n",
	           *temp = is_unsigned ? temp_unsigned : temp_signed;
	if ((tmp = asf_op_get_src(&bytes, src)) == NULL)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 2 + tmp->len);
	snprintf(s->s, s->len, temp, asf_suffix_get(bytes), tmp->s);
	return s;
}

int asf_op_mul(struct expr *e)
{
	enum ASF_REGS dest = ASF_OP_RESULT_REG;
	struct asf_val multiplicand = {};
	struct object_node *node = NULL;
	str *tmp = NULL;
	if (asf_op_try_push_prev_expr_result(e, dest) == TRY_RESULT_FAULT)
		return 1;
	if (asf_op_store_val(e->vall, &dest))
		return 1;
	if (asf_val_get(e->valr, &multiplicand))
		goto err_unsupport_type;
	if (asf_op_handle_expr(&tmp, e, &multiplicand.reg, dest))
		return 1;
	node = malloc(sizeof(*node));
	if ((node->s = asf_inst_op_mul(&multiplicand, 2)) == NULL)
		goto err_free_node;
	if (tmp != NULL) {
		str_append(tmp, node->s->len, node->s->s);
		str_free(node->s);
		node->s = tmp;
	}
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	*asf_regs[ASF_OP_RESULT_REG].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	return 0;
err_unsupport_type:
	printf("amc[backend.asf:%s]: asf_op_mul: Unsupport type\n", __FILE__);
	return 1;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return 1;
}

str *asf_inst_op_mul(struct asf_val *src, int is_unsigned)
{
	if (is_unsigned == 2)
		is_unsigned = asf_val_is_unsigned(src);
	if (src->type == ASF_VAL_MEM || src->type == ASF_VAL_REG)
		return mul_mem_or_reg_to_reg(src, is_unsigned);
	if (src->type == ASF_VAL_IMM)
		return mul_imm_to_reg(&src->imm, is_unsigned);
	printf("amc[backend.asf:%s]: asf_inst_op_mul: "
			"Unsupport type\n", __FILE__);
	return NULL;
}
