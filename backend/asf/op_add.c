/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/bytes.h"
#include "include/op.h"
#include "include/suffix.h"
#include <stdlib.h>
#include <string.h>

static str *add_imm_to_mem_or_reg(struct asf_imm *src, struct asf_val *dest);
static str *add_mem_to_reg(struct asf_mem *src, enum ASF_REGS dest);
static str *add_reg_to_mem_or_reg(enum ASF_REGS src, struct asf_val *dest);

str *add_imm_to_mem_or_reg(struct asf_imm *src, struct asf_val *dest)
{
	enum ASF_BYTES bytes;
	str *s = NULL, *tmp = NULL;
	const char *temp = "add%c $%lld, %s\n";
	if ((tmp = asf_op_get_dest(&bytes, dest)) == NULL)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 6 + tmp->len + sllen(src->iq));
	snprintf(s->s, s->len, temp, asf_suffix_get(bytes),
			src->iq,
			tmp->s);
	return s;
}

str *add_mem_to_reg(struct asf_mem *src, enum ASF_REGS dest)
{
	str *s = NULL, *tmp = NULL;
	const char *temp = "add%c %s, %%%s\n";
	if ((tmp = asf_mem_get_str(src)) == NULL)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 2 + tmp->len);
	snprintf(s->s, s->len, temp, asf_suffix_get(asf_regs[dest].bytes),
			tmp->s,
			asf_regs[dest].name);
	str_free(tmp);
	return s;
}

str *add_reg_to_mem_or_reg(enum ASF_REGS src, struct asf_val *dest)
{
	enum ASF_BYTES bytes;
	str *s = NULL, *tmp = NULL;
	const char *temp = "add%c %%%s, %s\n";
	if ((tmp = asf_op_get_dest(&bytes, dest)) == NULL)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 2 + tmp->len);
	snprintf(s->s, s->len, temp, asf_suffix_get(bytes),
			asf_regs[src].name,
			tmp->s);
	str_free(tmp);
	return s;
}

int asf_op_add(struct expr *e)
{
	struct asf_val addend = {},
	               augend = {
		.type = ASF_VAL_REG,
		.reg = ASF_OP_RESULT_REG
	};
	struct object_node *node = NULL;
	str *tmp = NULL;
	if (asf_op_try_push_prev_expr_result(e, augend.reg)
			== TRY_RESULT_FAULT)
		return 1;
	if (asf_op_store_val(e->vall, &augend.reg))
		return 1;
	if (asf_val_get(e->valr, &addend))
		goto err_unsupport_type;
	if (asf_op_handle_expr(&tmp, e, &addend.reg, augend.reg))
		return 1;
	node = malloc(sizeof(*node));
	if ((node->s = asf_inst_op_add(&addend, &augend)) == NULL)
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
	printf("amc[backend.asf:%s]: asf_op_add: Unsupport type\n", __FILE__);
	return 1;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return 1;
}

str *asf_inst_op_add(struct asf_val *src, struct asf_val *dest)
{
	switch (src->type) {
	case ASF_VAL_IMM:
		return add_imm_to_mem_or_reg(&src->imm, dest);
		break;
	case ASF_VAL_MEM:
		return add_mem_to_reg(&src->mem, dest->reg);
		break;
	case ASF_VAL_REG:
		return add_reg_to_mem_or_reg(src->reg, dest);
		break;
	default: break;
	}
	printf("amc[backend.asf:%s]: asf_inst_op_add: Unsupport type\n",
			__FILE__);
	return NULL;
}
