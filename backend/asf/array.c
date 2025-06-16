#include "include/asf.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/stack.h"
#include "include/suffix.h"
#include "include/val.h"
#include "../../include/backend/object.h"
#include <stdlib.h>
#include <string.h>

static int array_elem_push(yz_val *val);
static int array_elem_push_empty(yz_val *type);
static str *array_get_elem_from_imm(struct asf_stack_element *base,
		struct asf_imm *src);
static str *array_get_elem_from_mem(struct asf_stack_element *base,
		struct asf_stack_element *src);
static str *array_get_elem_from_reg(struct asf_stack_element *base,
		enum ASF_REGS src);

int array_elem_push(yz_val *val)
{
	struct object_node *node = NULL;
	if (val == NULL)
		return array_elem_push_empty(val);
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_push(val)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: array_elem_push: "
			"Get instruction failed!\n",
			__FILE__);
err_free_node:
	free(node);
	return 1;
}

int array_elem_push_empty(yz_val *val)
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
	printf("amc[backend.asf:%s]: array_elem_push_empty: "
			"Get instruction failed!\n",
			__FILE__);
err_free_node:
	free(node);
	return 1;
}

str *array_get_elem_from_imm(struct asf_stack_element *base,
		struct asf_imm *src)
{
	enum ASF_REGS dest = asf_reg_get(base->bytes);
	int offset = 0;
	str *s = str_new();
	const char *temp = "mov%c -%lld(%%rbp), %%%s\n";
	offset = base->bytes * src->iq;
	str_expand(s, strlen(temp) - 4 + ullen(offset));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(base->bytes),
			base->addr - offset,
			asf_regs[dest].name);
	return s;
}

str *array_get_elem_from_mem(struct asf_stack_element *base,
		struct asf_stack_element *src)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, src, &dest)) == NULL)
		goto err_free_node;
	return array_get_elem_from_reg(base, dest);
err_free_node:
	free(node);
	return NULL;
}

str *array_get_elem_from_reg(struct asf_stack_element *base, enum ASF_REGS src)
{
	enum ASF_REGS dest = asf_reg_get(base->bytes);
	str *s = str_new();
	const char *temp = "mov%c -%lld(%%rbp,%%%s,%lld), %%%s\n";
	str_expand(s, strlen(temp) - 8 + ullen(base->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(base->bytes),
			base->addr,
			asf_regs[src - asf_reg_get(asf_regs[src].bytes)].name,
			base->bytes,
			asf_regs[dest].name);
	return s;
}

int asf_array_def(backend_symbol_status **raw_sym_stat, yz_val **vs, int len)
{
	for (int i = len - 1; i != -1; i--) {
		if (array_elem_push(vs[i]))
			return 1;
	}
	if (asf_identifier_reg(raw_sym_stat, asf_stack_top))
		goto err_identifier_reg_failed;
	return 0;
err_identifier_reg_failed:
	printf("amc[backend.asf]: asf_array_def: "
			"Identifier register failed!\n");
	return 1;
}

int asf_array_get_elem(backend_symbol_status *raw_sym_stat, yz_val *offset)
{
	struct asf_stack_element *base = NULL;
	struct object_node *node = NULL;
	struct asf_val val = {};
	if ((base = raw_sym_stat) == NULL)
		goto err_identifier_not_found;
	if (asf_val_get(offset, &val))
		return 1;
	node = malloc(sizeof(*node));
	if (val.type == ASF_VAL_IMM) {
		if ((node->s = array_get_elem_from_imm(base, &val.imm))
				== NULL)
			goto err_inst_failed;
	} else if (val.type == ASF_VAL_MEM) {
		if ((node->s = array_get_elem_from_mem(base, val.mem)) == NULL)
			goto err_inst_failed;
	} else if (val.type == ASF_VAL_REG) {
		if ((node->s = array_get_elem_from_reg(base, val.reg)) == NULL)
			goto err_inst_failed;
	}
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_identifier_not_found:
	printf("amc[backend.asf]: asf_array_get_elem: "
			"Identifier not found!\n");
	return 1;
err_inst_failed:
	printf("amc[backend.asf]: asf_array_get_elem: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
err_free_node_and_str:
	str_free(node->s);
	goto err_free_node;
}
