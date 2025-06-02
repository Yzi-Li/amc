#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/stack.h"
#include "include/suffix.h"
#include "../../include/backend/object.h"

static int array_elem_push(yz_val *val);
static int array_elem_push_empty(yz_val *type);
static int array_get_elem_from_imm(struct asf_stack_element *base,
		yz_val *val);
static int array_get_elem_from_reg(struct asf_stack_element *base,
		enum ASF_REGS src);
static int array_get_elem_from_sym(struct asf_stack_element *base,
		struct symbol *sym);

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

int array_get_elem_from_imm(struct asf_stack_element *base, yz_val *val)
{
	enum ASF_REGS dest = asf_reg_get(base->bytes);
	struct object_node *node = NULL;
	int offset = 0;
	const char *temp = "mov%c -%lld(%%rbp), %%%s\n";
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	offset = base->bytes * val->l;
	str_expand(node->s, strlen(temp) - 4 + ullen(offset));
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(base->bytes),
			base->addr - offset,
			asf_regs[dest].name);
	return 0;
err_free_node:
	free(node);
	return 1;
}

int array_get_elem_from_reg(struct asf_stack_element *base, enum ASF_REGS src)
{
	enum ASF_REGS dest = asf_reg_get(base->bytes);
	struct object_node *node = NULL;
	const char *temp = "mov%c -%lld(%%rbp,%%%s,%lld), %%%s\n";
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 8 + ullen(base->addr));
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(base->bytes),
			base->addr,
			asf_regs[src].name,
			base->bytes,
			asf_regs[dest].name);
	return 0;
err_free_node:
	free(node);
	return 1;
}

int array_get_elem_from_sym(struct asf_stack_element *base, struct symbol *sym)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	char *name = NULL;
	struct object_node *node = NULL;
	struct asf_stack_element *src = NULL;
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			return 1;
		return array_get_elem_from_reg(base,
				asf_call_arg_regs[sym->argc - 2]);
	} else if (sym->args == NULL && sym->argc == 1) {
		name = str2chr(sym->name, sym->name_len);
		if ((src = asf_identifier_get(name)) == NULL)
			goto err_identifier_not_found;
		free(name);
		node = malloc(sizeof(*node));
		if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
			goto err_free_node;
		if ((node->s = asf_inst_mov(ASF_MOV_M2R, src, &dest)) == NULL)
			goto err_inst_failed;
	}
	return array_get_elem_from_reg(base, ASF_REG_RAX);
err_identifier_not_found:
	printf("amc[backend.asf:%s]: array_get_elem_from_sym: "
			"Identifier: \"%s\" not found!\n", name, __FILE__);
	free(name);
	return 1;
err_inst_failed:
	printf("amc[backend.asf:%s]: array_get_elem_from_sym: "
			"Instruction failed!\n", __FILE__);
err_free_node:
	free(node);
	return 1;
}

int asf_array_def(char *name, yz_val **vs, int len,
		backend_scope_status *raw_status)
{
	for (int i = len - 1; i != -1; i--) {
		if (array_elem_push(vs[i]))
			return 1;
	}
	if (asf_identifier_reg(name, asf_stack_top, raw_status))
		goto err_identifier_reg_failed;
	return 0;
err_identifier_reg_failed:
	printf("amc[backend.asf]: asf_array_def: "
			"Identifier register failed!\n");
	return 1;
}

int asf_array_get_elem(char *name, yz_val *offset)
{
	struct asf_stack_element *base = NULL;
	if ((base = asf_identifier_get(name)) == NULL)
		goto err_identifier_not_found;
	if (offset->type == AMC_SYM) {
		return array_get_elem_from_sym(base, offset->v);
	} else if (offset->type == AMC_EXPR) {
		return array_get_elem_from_reg(base, ASF_REG_RAX);
	} else if (YZ_IS_DIGIT(offset->type)) {
		return array_get_elem_from_imm(base, offset);
	}
	return 0;
err_identifier_not_found:
	printf("amc[backend.asf]: asf_array_get_elem: "
			"Identifier not found!\n");
	return 1;
}
