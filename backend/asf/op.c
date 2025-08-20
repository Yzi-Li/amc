/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/bytes.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/register.h"
#include "include/stack.h"
#include "../../include/expr.h"
#include <stdlib.h>
#include <string.h>

static int op_init_obj_node(struct object_node *parent,
		struct object_node *node);

int op_init_obj_node(struct object_node *parent, struct object_node *node)
{
	if (node == NULL)
		return 0;
	if (parent == NULL) {
		if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
			return 1;
		return 0;
	}
	if (object_insert(node, parent->prev, parent))
		return 1;
	return 0;
}

int asf_op_clean_reg(struct object_node *parent, enum ASF_REGS reg)
{
	struct object_node *node = malloc(sizeof(*node));
	if (op_init_obj_node(parent, node))
		goto err_free_node;
	if ((node->s = asf_reg_clean(reg)) == NULL)
		goto err_inst_failed;
	return 0;
err_free_node:
	free(node);
	return 1;
err_inst_failed:
	printf("amc[backend.asf:%s]: asf_op_clean_reg: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

str *asf_op_get_dest(enum ASF_BYTES *bytes, struct asf_val *dest)
{
	if (dest->type == ASF_VAL_MEM) {
		*bytes = dest->mem.bytes;
		return asf_mem_get_str(&dest->mem);
	} else if (dest->type == ASF_VAL_REG) {
		*bytes = asf_regs[dest->reg].bytes;
		return asf_reg_get_str(&asf_regs[dest->reg]);
	}
	return NULL;
}

str *asf_op_get_src(enum ASF_BYTES *bytes, struct asf_val *src)
{
	if (src->type == ASF_VAL_MEM) {
		*bytes = src->mem.bytes;
		return asf_mem_get_str(&src->mem);
	} else if (src->type == ASF_VAL_REG) {
		*bytes = asf_regs[src->reg].bytes;
		return asf_reg_get_str(&asf_regs[src->reg]);
	}
	return NULL;
}

int asf_op_handle_expr(str **result, struct expr *e, enum ASF_REGS *src,
		enum ASF_REGS dest)
{
	enum YZ_TYPE lraw = e->vall->type.type, rraw = e->valr->type.type;
	if (lraw == AMC_EXPR
			&& (rraw == AMC_SYM
				&& e->valr->sym->type == SYM_FUNC)) {
		*src = ASF_OP_OPERAND_REG + asf_reg_get(
				asf_yz_type2bytes(&e->valr->type));
		*result = asf_inst_pop(ASF_OP_OPERAND_REG);
		if (*result == NULL)
			return 1;
	} else if (rraw == AMC_EXPR
			|| (rraw == AMC_SYM
				&& e->valr->sym->type == SYM_FUNC)) {
		*result = asf_op_handle_expr_and_expr(src, dest);
		if (*result == NULL)
			return 1;
	}
	return 0;
}

str *asf_op_handle_expr_and_expr(enum ASF_REGS *src, enum ASF_REGS dest)
{
	str *s = NULL, *tmp = NULL;
	*src = ASF_OP_OPERAND_REG + asf_reg_get(asf_regs[dest].bytes);
	s = asf_inst_mov_r2r(ASF_OP_RESULT_REG, ASF_OP_OPERAND_REG);
	if (s == NULL)
		return NULL;
	if ((tmp = asf_inst_pop(ASF_OP_RESULT_REG)) == NULL)
		goto err_free_s;
	str_append(s, tmp->len, tmp->s);
	str_free(tmp);
	return s;
err_free_s:
	str_free(s);
	return NULL;
}

int asf_op_save_reg(struct object_node *parent, enum ASF_REGS reg)
{
	struct object_node *node = malloc(sizeof(*node));
	if (op_init_obj_node(parent, node))
		goto err_free_node;
	if ((node->s = asf_inst_push_reg(reg)) == NULL)
		goto err_inst_failed;
	return 0;
err_free_node:
	free(node);
	return 1;
err_inst_failed:
	printf("amc[backend.asf:%s]: asf_op_save_reg: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

int asf_op_store_val(yz_val *val, enum ASF_REGS *dest)
{
	struct object_node *node = NULL;
	struct asf_val v = {};
	if (asf_val_get(val, &v))
		goto err_unsupport_type;
	*dest += asf_reg_get(asf_yz_type2bytes(&val->type));
	node = malloc(sizeof(*node));
	switch (v.type) {
	case ASF_VAL_IMM:
		if ((node->s = asf_inst_mov_i2r(&v.imm, *dest)) == NULL)
			goto err_free_node;
		break;
	case ASF_VAL_MEM:
		if ((node->s = asf_inst_mov_m2r(&v.mem, *dest)) == NULL)
			goto err_free_node;
		break;
	case ASF_VAL_REG:
		if (v.reg == *dest) {
			free(node);
			return 0;
		}
		if ((node->s = asf_inst_mov_r2r(v.reg, *dest)) == NULL)
			goto err_free_node;
		break;
	default: goto err_unsupport_type; break;
	}
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		return 1;
	return 0;
err_unsupport_type:
	printf("amc[backend.asf:%s]: asf_op_store_val: Unsupport type\n",
			__FILE__);
	return 1;
err_free_node:
	free(node);
	return 1;
}

// FIXME: Don't push prev expr result in next line.
//        e.g:
//          (1 + 1) + [func 1]
//          [func (1 - 7)] ; <-- HERE will push prev expr
enum TRY_RESULT
asf_op_try_push_prev_expr_result(struct expr *e, enum ASF_REGS reg)
{
	struct object_node *node = NULL;
	if (*asf_regs[reg].purpose == ASF_REG_PURPOSE_NULL)
		return TRY_RESULT_NOT_HANDLED;
	if (e->vall->type.type == AMC_EXPR)
		return TRY_RESULT_NOT_HANDLED;
	if (e->vall->type.type == AMC_SYM && e->vall->sym->type == SYM_FUNC)
		return TRY_RESULT_NOT_HANDLED;
	node = malloc(sizeof(*node));
	if ((node->s = asf_inst_push_reg(reg)) == NULL)
		goto err_free_node;
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	*asf_regs[reg].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	return TRY_RESULT_HANDLED;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return TRY_RESULT_FAULT;
}
