/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/cmp.h"
#include "include/jmp.h"
#include "include/label.h"
#include "include/op.h"
#include "include/suffix.h"
#include "../../include/backend/object.h"
#include <stdlib.h>
#include <string.h>

static int cmp_and_jmp(struct expr *e, enum ASF_JMP_TYPE jmp_type);
static str *cmp_imm_with_mem_or_reg(struct asf_imm *src, struct asf_val *dest);
static int cmp_jmp_inst_append(label_id label, str *label_str,
		enum ASF_JMP_TYPE jmp_type);
static str *cmp_mem_with_reg(struct asf_mem *src, enum ASF_REGS dest);
static str *cmp_reg_with_mem_or_reg(enum ASF_REGS src, struct asf_val *dest);

int cmp_and_jmp(struct expr *e, enum ASF_JMP_TYPE jmp_type)
{
	label_id label = -1;
	str *label_str = NULL;
	struct object_node *node = NULL;
	struct asf_val src = {},
	               dest = {
		.type = ASF_VAL_REG,
		.reg = ASF_OP_RESULT_REG
	};
	if ((label = asf_label_alloc()) == -1)
		goto err_label_alloc_failed;
	if (asf_op_try_push_prev_expr_result(e, ASF_OP_RESULT_REG) < 0)
		return 1;
	if (asf_op_store_val(e->vall, &dest.reg))
		return 1;
	if (asf_val_get(e->valr, &src))
		goto err_unsupport_type;
	node = malloc(sizeof(*node));
	if ((node->s = asf_inst_cmp(&src, &dest)) == NULL)
		goto err_free_node;
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	if ((label_str = asf_label_get_str(label)) == NULL)
		goto err_label_get_str_failed;
	if (cmp_jmp_inst_append(label, label_str, jmp_type))
		return 1;
	return 0;
err_label_alloc_failed:
	printf("amc[backend.asf:%s]: cmp_and_jmp: Label alloc failed!\n",
			__FILE__);
	return 1;
err_unsupport_type:
	printf("amc[backend.asf:%s]: cmp_and_jmp: Unsupport type\n", __FILE__);
	return 1;
err_label_get_str_failed:
	printf("amc[backend.asf:%s]: cmp_and_jmp: Label get str failed!\n",
			__FILE__);
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return 1;
}

str *cmp_imm_with_mem_or_reg(struct asf_imm *src, struct asf_val *dest)
{
	enum ASF_BYTES bytes;
	str *s = NULL, *tmp = NULL;
	const char *temp = "cmp%c $%lld, %s\n";
	if ((tmp = asf_op_get_dest(&bytes, dest)) == NULL)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 6 + ullen(src->iq) + tmp->len);
	snprintf(s->s, s->len, temp, asf_suffix_get(bytes),
			src->iq,
			tmp->s);
	return s;
}

int cmp_jmp_inst_append(label_id label, str *label_str,
		enum ASF_JMP_TYPE jmp_type)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_jmp(jmp_type, label_str->s, label_str->len))
			== NULL)
		goto err_inst_jmp;
	return 0;
err_free_node:
	free(node);
	return 1;
err_inst_jmp:
	printf("amc[backend.asf:%s]: cmp_jmp_inst_append: "
			"Get instruction 'jmp' failed!\n", __FILE__);
	str_free(label_str);
	goto err_free_node;
}

str *cmp_mem_with_reg(struct asf_mem *src, enum ASF_REGS dest)
{
	str *s = NULL, *tmp = NULL;
	const char *temp = "cmp%c %s, %%%s\n";
	if ((tmp = asf_mem_get_str(src)) == NULL)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 2 + tmp->len);
	snprintf(s->s, s->len, temp, asf_suffix_get(asf_regs[dest].bytes),
			tmp->s,
			asf_regs[dest].name);
	return s;
}

str *cmp_reg_with_mem_or_reg(enum ASF_REGS src, struct asf_val *dest)
{
	enum ASF_BYTES bytes;
	str *s = NULL, *tmp = NULL;
	const char *temp = "cmp%c %%%s, %s\n";
	if ((tmp = asf_op_get_dest(&bytes, dest)) == NULL)
		return NULL;
	s = str_new();
	str_expand(s, strlen(temp) - 2 + tmp->len);
	snprintf(s->s, s->len, temp, asf_suffix_get(bytes),
			asf_regs[src].name,
			tmp->s);
	return s;
}

int asf_op_eq(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_NE);
}

int asf_op_ne(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_EQ);
}

int asf_op_le(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_GT);
}

int asf_op_lt(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_GE);
}

int asf_op_ge(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_LT);
}

int asf_op_gt(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_LE);
}

str *asf_inst_cmp(struct asf_val *src, struct asf_val *dest)
{
	switch (src->type) {
	case ASF_VAL_IMM:
		return cmp_imm_with_mem_or_reg(&src->imm, dest);
		break;
	case ASF_VAL_MEM:
		return cmp_mem_with_reg(&src->mem, dest->reg);
		break;
	case ASF_VAL_REG:
		return cmp_reg_with_mem_or_reg(src->reg, dest);
		break;
	default: break;
	}
	return NULL;
}
