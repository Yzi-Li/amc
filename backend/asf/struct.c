#include "include/asf.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/stack.h"
#include "../../include/backend/object.h"

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
	struct asf_stack_element *base = raw_sym_stat, *cur;
	enum ASF_REGS dest = ASF_REG_RAX;
	struct object_node *node = NULL;
	cur = base;
	for (int i = 0; i < index; i++) {
		cur = cur->next;
		if (cur == NULL)
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
