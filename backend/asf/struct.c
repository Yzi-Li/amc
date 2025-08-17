/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/op_val.h"
#include "include/stack.h"
#include "include/struct.h"
#include "../../include/backend/object.h"
#include "../../include/ptr.h"
#include <stdlib.h>
#include <string.h>

static int struct_elem_push(yz_val *val);
static int struct_elem_push_empty(yz_val *val);

int struct_elem_push(yz_val *val)
{
	struct object_node *node = NULL;
	if (val == NULL)
		return struct_elem_push_empty(val);
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_push(val)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: struct_elem_push: "
			"Get instruction failed!\n",
			__FILE__);
err_free_node:
	free(node);
	return 1;
}

int struct_elem_push_empty(yz_val *val)
{
	struct asf_imm imm = {
		.type = asf_yz_type2bytes(&val->type),
		.iq = 0
	};
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_push_imm(&imm)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: struct_elem_push_empty: "
			"Get instruction failed!\n",
			__FILE__);
err_free_node:
	free(node);
	return 1;
}

int asf_struct_def(backend_symbol_status *raw_sym_stat, yz_val **vs, int len)
{
	for (int i = len - 1; i >= 0; i--) {
		if (struct_elem_push(vs[i]))
			return 1;
	}
	if (asf_identifier_reg(raw_sym_stat, asf_stack_top))
		goto err_identifier_reg_failed;
	return 0;
err_identifier_reg_failed:
	printf("amc[backend.asf]: asf_struct_def: "
			"Identifier register failed!\n");
	return 1;
}

struct asf_stack_element *asf_struct_get_elem(struct asf_stack_element *base,
		int index)
{
	struct asf_stack_element *result = base;
	for (int i = 0; i < index; i++) {
		if ((result = result->prev) == NULL)
			return NULL;
	}
	return result;
}

int asf_struct_set_elem(struct symbol *ident, int index, yz_val *val,
		enum OP_ID mode)
{
	struct asf_stack_element *dest =
		asf_struct_get_elem(ident->backend_status, index);
	struct asf_mem mem = {};
	struct object_node *node = NULL;
	if (dest == NULL)
		return 1;
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_identifier_set(asf_stack_element2mem(dest, &mem),
					mode, val)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_var_set: Get instruction failed!\n");
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_struct_set_elem_from_ptr(struct symbol *ident, int index, yz_val *val,
		enum OP_ID mode)
{
	return 0;
}

int asf_op_extract_struct_elem(yz_extract_val *val)
{
	struct asf_stack_element *cur =
		asf_struct_get_elem(val->sym->backend_status, val->index);
	enum ASF_REGS dest = ASF_REG_RAX;
	struct asf_mem mem = {};
	struct object_node *node = NULL;
	if (cur == NULL)
		return 1;
	dest = asf_reg_get(cur->bytes);
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_mov_m2r(asf_stack_element2mem(cur, &mem),
					dest)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_struct_get_elem: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int asf_op_extract_struct_elem_from_ptr(yz_extract_val *val)
{
	enum ASF_REGS dest = ASF_REG_RAX, src = ASF_REG_RAX;
	str *inst = NULL, *tmp = NULL;
	struct object_node *node = NULL;
	int offset = 0;
	struct yz_struct *s = ((yz_ptr_type*)val->sym->result_type.v)->ref.v;
	struct asf_mem src_operand = {};
	if (val->sym->type == SYM_FUNC_ARG) {
		if (val->sym->argc > asf_call_arg_regs_len)
			return 1;
		src = asf_call_arg_regs[val->sym->argc];
	} else if (val->sym->type == SYM_IDENTIFIER) {
		asf_stack_element2mem(val->sym->backend_status, &src_operand);
		if ((tmp = asf_inst_mov_m2r(&src_operand, src)) == NULL)
			return 1;
	} else {
		return 1;
	}
	dest = asf_reg_get(asf_yz_type2bytes(&val->elem->result_type));
	node = malloc(sizeof(*node));
	for (int i = 0; i < val->index; i++)
		offset += asf_yz_type2bytes(&s->elems[i]->result_type);
	src_operand.addr = src;
	src_operand.bytes = asf_regs[dest].bytes;
	src_operand.offset = offset;
	if ((inst = asf_inst_mov_m2r(&src_operand, dest)) == NULL)
		goto err_inst_failed;
	if (val->sym->type == SYM_IDENTIFIER) {
		node->s = tmp;
		str_append(node->s, inst->len, inst->s);
		str_free(inst);
	} else {
		node->s = inst;
	}
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_free_node_and_str:
	str_free(node->s);
	free(node);
	return 1;
err_inst_failed:
	if (val->sym->type == SYM_IDENTIFIER)
		str_free(tmp);
	free(node);
	return 1;
}
