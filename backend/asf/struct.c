#include "include/asf.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/stack.h"
#include "../../include/backend/object.h"
#include <stdlib.h>

static int struct_elem_push(yz_val *val);
static int struct_elem_push_empty(yz_val *val);

int struct_elem_push(yz_val *val)
{
	struct object_node *node = NULL;
	if (val == NULL)
		return struct_elem_push_empty(val);
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
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
		.type = asf_yz_type2bytes(val),
		.iq = 0
	};
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
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
	if (struct_elem_push(vs[0]))
		return 1;
	if (asf_identifier_reg(raw_sym_stat, asf_stack_top))
		goto err_identifier_reg_failed;
	for (int i = 1; i < len; i++) {
		if (struct_elem_push(vs[i]))
			return 1;
	}
	return 0;
err_identifier_reg_failed:
	printf("amc[backend.asf]: asf_struct_def: "
			"Identifier register failed!\n");
	return 1;
}

int asf_struct_get_elem(backend_symbol_status *raw_sym_stat, yz_struct *src,
		int index)
{
	struct asf_stack_element *cur = raw_sym_stat;
	enum ASF_REGS dest = ASF_REG_RAX;
	struct object_node *node = NULL;
	for (int i = 0; i < index; i++) {
		if ((cur = cur->next) == NULL)
			return 1;
	}
	dest = asf_reg_get(cur->bytes);
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, cur, &dest)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_struct_get_elem: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int asf_struct_set_elem(struct symbol *sym, int index, yz_val *val,
		enum OP_ID mode)
{
	struct asf_stack_element *dest = sym->backend_status;
	struct object_node *node = NULL;
	// FIXME: Will remove 'lea' instruction
	//        when value is a memory address from 'op_unary_get_addr'.
	if (object_remove(&objs[cur_obj][ASF_OBJ_TEXT]))
		return 1;
	for (int i = 0; i < index; i++) {
		if ((dest = dest->next) == NULL)
			return 1;
	}
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_identifier_set(dest, mode, val)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_var_set: Get instruction failed!\n");
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
