#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/register.h"
#include "include/stack.h"
#include "../../include/backend/object.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"
#include "../../utils/utils.h"

static int func_call_basic_args(str *s, yz_val **vs, int vlen);
static int func_call_ext_args(str *s, yz_val **vs, int vlen);
static int func_call_push_arg(str *s, int index, yz_val *v);
static int func_call_push_arg_expr(str *s, int index, struct expr *expr);
static int func_call_push_arg_identifier(str *s, int index,
		struct symbol *sym);
static int func_call_push_arg_imm(str *s, int index, yz_val *v);
static int func_call_push_arg_sym(str *s, int index, struct symbol *sym);
static int func_call_set_stack_top(struct object_node *parent);
static void func_ret_clean_stack();
static int func_ret_expr(struct expr *expr);
static int func_ret_identifier(struct object_node *node, struct symbol *sym);
static int func_ret_imm(yz_val *v);
static int func_ret_main(yz_val *v, str *s);
static int func_ret_sym(struct symbol *sym);
static int func_ret_val(yz_val *v);

int func_call_basic_args(str *s, yz_val **vs, int vlen)
{
	for (int i = 0; i < vlen; i++) {
		if (func_call_push_arg(s, i, vs[i]))
			return 1;
	}
	return 0;
}

int func_call_ext_args(str *s, yz_val **vs, int vlen)
{
	for (int i = vlen - 1; i != 0; i--) {
		if (func_call_push_arg(s, i, vs[i]))
			return 1;
	}
	return 0;
}

int func_call_push_arg(str *s, int index, yz_val *v)
{
	if (v->type == AMC_SYM) {
		return func_call_push_arg_sym(s, index, v->v);
	} else if (v->type == AMC_EXPR) {
		return func_call_push_arg_expr(s, index, v->v);
	} else if (YZ_IS_DIGIT(v->type)) {
		return func_call_push_arg_imm(s, index, v);
	}
	printf("amc[backend.asf]: func_call_push_arg: "
			"Unsupport argument type: \"%s\"\n",
			yz_get_type_name(v));
	return 1;
}

int func_call_push_arg_expr(str *s, int index, struct expr *expr)
{
	enum ASF_REGS dest = asf_call_arg_regs[index],
	              src = ASF_REG_RAX;
	str *tmp = NULL;
	src = asf_reg_get(asf_yz_type_raw2bytes(*expr->sum_type));
	if (index > asf_call_arg_regs_len) {
		tmp = asf_inst_push_reg(src);
	} else {
		dest += src;
		tmp = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	}
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int func_call_push_arg_identifier(str *s, int index, struct symbol *sym)
{
	enum ASF_REGS dest = asf_call_arg_regs[index];
	char *name = str2chr(sym->name, sym->name_len);
	str *tmp = NULL;
	struct asf_stack_element *src = asf_identifier_get(name);
	if (index > asf_call_arg_regs_len) {
		printf("amc[backend.asf]: func_call_push_arg_identifier: "
				"Unsupport syntax!\n");
		return 1;
	} else {
		dest += asf_reg_get(src->bytes);
		tmp = asf_inst_mov(ASF_MOV_M2R, src, &dest);
	}
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int func_call_push_arg_imm(str *s, int index, yz_val *v)
{
	enum ASF_REGS dest = asf_call_arg_regs[index];
	struct asf_imm imm = {
		.type = asf_yz_type2bytes(v),
		.iq = v->l
	};
	str *tmp = NULL;
	if (index > asf_call_arg_regs_len) {
		tmp = asf_inst_push_imm(&imm);
	} else {
		dest += asf_reg_get(imm.type);
		tmp = asf_inst_mov(ASF_MOV_I2R, &imm, &dest);
	}
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int func_call_push_arg_sym(str *s, int index, struct symbol *sym)
{
	enum ASF_REGS dest = ASF_REG_RDI,
	              offset_base = ASF_REG_RAX,
	              src = ASF_REG_RAX;
	str *tmp = NULL;
	if (sym->args == NULL && sym->argc == 1)
		return func_call_push_arg_identifier(s, index, sym);
	offset_base = asf_reg_get(asf_yz_type2bytes(&sym->result_type));
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			return 1;
		src = offset_base + asf_call_arg_regs[sym->argc - 2];
	} else {
		src = offset_base;
	}
	if (index > asf_call_arg_regs_len) {
		tmp = asf_inst_push_reg(src);
	} else {
		dest = offset_base + asf_call_arg_regs[index];
		if (dest == src)
			return 0;
		tmp = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	}
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int func_call_set_stack_top(struct object_node *parent)
{
	struct object_node *node = malloc(sizeof(*node));
	const char *temp = "subq $%lld, %%rsp\n";
	if (object_insert(node, parent->prev, parent))
		return 1;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 4
			+ ullen(asf_stack_top->addr));
	snprintf(node->s->s, node->s->len, temp, asf_stack_top->addr);
	return 0;
}

void func_ret_clean_stack()
{
	struct asf_stack_element *cur = asf_stack_top, *nex;
	while (cur != NULL) {
		nex = cur->prev;
		free(cur);
		cur = nex;
	}
	asf_stack_top = NULL;
	asf_identifier_free_id(0);
	return;
}

int func_ret_expr(struct expr *expr)
{
	enum ASF_REGS reg = asf_reg_get(
			asf_yz_type_raw2bytes(*expr->sum_type));
	if (*asf_regs[reg].purpose != ASF_REG_PURPOSE_EXPR_RESULT)
		return 1;
	*asf_regs[reg].purpose = ASF_REG_PURPOSE_NULL;
	return 0;
}

int func_ret_identifier(struct object_node *node, struct symbol *sym)
{
	char *name = str2chr(sym->name, sym->name_len);
	enum ASF_REGS dest = asf_reg_get(asf_yz_type2bytes(&sym->result_type));
	struct asf_stack_element *src = asf_identifier_get(name);
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_all;
	if (src == NULL)
		goto err_free_all;
	node->s = asf_inst_mov(ASF_MOV_M2R, src, &dest);
	free(name);
	return 0;
err_free_all:
	free(name);
	str_free(node->s);
	free(node);
	return 1;
}

int func_ret_imm(yz_val *v)
{
	struct asf_imm imm = {};
	struct object_node *node = malloc(sizeof(*node));
	enum ASF_REGS reg = ASF_REG_RAX;
	imm.type = asf_yz_type2bytes(v);
	imm.iq = v->l;
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	reg = asf_reg_get(imm.type);
	node->s = asf_inst_mov(ASF_MOV_I2R, &imm, &reg);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int func_ret_main(yz_val *v, str *s)
{
	str *tmp = NULL;
	if ((tmp = asf_inst_syscall(60, 1, &v)) == NULL)
		goto err_free_tmp;
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
err_free_tmp:
	str_free(tmp);
	return 1;
}

int func_ret_sym(struct symbol *sym)
{
	struct object_node *node = NULL;
	enum ASF_REGS dest = ASF_REG_RAX,
	              src = ASF_REG_RDI;
	if (sym->args == NULL && sym->argc == 1)
		return func_ret_identifier(node, sym);
	if (sym->args != NULL && sym->argc != 0)
		return 0;
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	dest = asf_reg_get(asf_yz_type2bytes(&sym->result_type));
	if (sym->argc - 2 > asf_call_arg_regs_len)
		goto err_free_node;
	src = asf_call_arg_regs[sym->argc - 2] + dest;
	node->s = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int func_ret_val(yz_val *v)
{
	if (v->type == AMC_EXPR) {
		return func_ret_expr(v->v);
	} else if (v->type == AMC_SYM) {
		return func_ret_sym(v->v);
	} else if (YZ_IS_DIGIT(v->type)) {
		return func_ret_imm(v);
	}
	return 1;
}

int asf_func_call(const char *name, yz_val *type, yz_val **vs, int vlen)
{
	struct object_node *node = malloc(sizeof(*node));
	int node_str_last = 0;
	int inst_len = 0;
	enum ASF_REGS reg = ASF_REG_RAX;
	const char *temp = "call %s\n";
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	reg = asf_reg_get(asf_yz_type2bytes(type));
	if (*asf_regs[reg].purpose != ASF_REG_PURPOSE_NULL)
		if (asf_op_save_reg(node, reg))
			goto err_free_node;
	*asf_regs[reg].purpose = ASF_REG_PURPOSE_FUNC_RESULT;
	if (func_call_basic_args(node->s, vs, vlen))
		goto err_free_node;
	if (vlen > asf_call_arg_regs_len) {
		if (func_call_ext_args(node->s,
				&vs[asf_call_arg_regs_len],
				vlen - asf_call_arg_regs_len))
			goto err_free_node;
	}
	if (asf_stack_top != NULL) {
		if (func_call_set_stack_top(node))
			goto err_free_node;
	}
	node_str_last = node->s->len;
	inst_len = strlen(temp) - 1 + strlen(name);
	str_expand(node->s, inst_len);
	snprintf(&node->s->s[node_str_last], inst_len, temp, name);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_func_def(const char *name, int len, yz_val *type)
{
	const char *temp =
		".globl %s\n"
		"%s:\n"
		"pushq %%rbp\n"
		"movq %%rsp, %%rbp\n";
	char *tmp_name = malloc(len + 1);
	struct object_node *node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	memcpy(tmp_name, name, len);
	tmp_name[len] = '\0';
	str_expand(node->s, strlen(temp) - 6 + (len * 2));
	snprintf(node->s->s, node->s->len, temp, tmp_name, tmp_name);
	free(tmp_name);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_func_ret(yz_val *v, int is_main)
{
	const char *temp =
		"popq %rbp\n"
		"ret\n";
	struct object_node *node = malloc(sizeof(*node));
	node->s = str_new();
	if (is_main) {
		if (func_ret_main(v, node->s))
			goto err_free_node;
	} else if (func_ret_val(v)) {
		goto err_free_node;
	}
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	str_append(node->s, strlen(temp), temp);
	func_ret_clean_stack();
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
