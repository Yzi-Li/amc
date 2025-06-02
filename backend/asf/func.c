#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "../../include/backend/object.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"
#include "../../utils/utils.h"
#include <stdlib.h>
#include <string.h>

static int func_call_set_stack_top(struct object_node *parent);
static int func_ret_expr(struct expr *expr, str **s);
static int func_ret_identifier(struct symbol *sym, str **s);
static int func_ret_imm(yz_val *v, str **s);
static int func_ret_main(yz_val *v, str **s);
static int func_ret_sym(struct symbol *sym, str **s);
static int func_ret_val(yz_val *v, str **s);

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

int func_ret_expr(struct expr *expr, str **s)
{
	enum ASF_REGS reg = asf_reg_get(
			asf_yz_type_raw2bytes(*expr->sum_type));
	if (*asf_regs[reg].purpose != ASF_REG_PURPOSE_EXPR_RESULT)
		return 1;
	*asf_regs[reg].purpose = ASF_REG_PURPOSE_NULL;
	*s = NULL;
	return 0;
}

int func_ret_identifier(struct symbol *sym, str **s)
{
	char *name = str2chr(sym->name, sym->name_len);
	enum ASF_REGS dest = asf_reg_get(asf_yz_type2bytes(&sym->result_type));
	struct asf_stack_element *src = asf_identifier_get(name);
	free(name);
	if (src == NULL)
		return 1;
	if ((*s = asf_inst_mov(ASF_MOV_M2R, src, &dest)) == NULL)
		goto err_inst_failed;
	(*s)->len -= 1;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: func_ret_identifier: "
			"Get instruction failed!\n", __FILE__);
	return 1;
}

int func_ret_imm(yz_val *v, str **s)
{
	struct asf_imm imm = {};
	enum ASF_REGS reg = ASF_REG_RAX;
	imm.type = asf_yz_type2bytes(v);
	imm.iq = v->l;
	reg = asf_reg_get(imm.type);
	if ((*s = asf_inst_mov(ASF_MOV_I2R, &imm, &reg)) == NULL)
		goto err_inst_failed;
	(*s)->len -= 1;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: func_ret_imm: Get instruction failed!\n",
			__FILE__);
	return 1;
}

int func_ret_main(yz_val *v, str **s)
{
	if ((*s = asf_inst_syscall(60, 1, &v)) == NULL)
		goto err_inst_failed;
	(*s)->len -= 1;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: func_ret_main: Get instruction failed!\n",
			__FILE__);
	return 1;
}

int func_ret_sym(struct symbol *sym, str **s)
{
	enum ASF_REGS dest = ASF_REG_RAX,
	              src = ASF_REG_RDI;
	if (sym->args == NULL && sym->argc == 1)
		return func_ret_identifier(sym, s);
	if (sym->args != NULL && sym->argc != 0)
		return 0;
	if (sym->argc - 2 > asf_call_arg_regs_len)
		return 1;
	dest = asf_reg_get(asf_yz_type2bytes(&sym->result_type));
	src = asf_call_arg_regs[sym->argc - 2] + dest;
	if ((*s = asf_inst_mov(ASF_MOV_R2R, &src, &dest)) == NULL)
		goto err_inst_failed;
	(*s)->len -= 1;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: func_ret_sym: Get instruction failed!\n",
			__FILE__);
	return 1;
}

int func_ret_val(yz_val *v, str **s)
{
	if (v->type == AMC_EXPR) {
		return func_ret_expr(v->v, s);
	} else if (v->type == AMC_SYM) {
		return func_ret_sym(v->v, s);
	} else if (YZ_IS_DIGIT(v->type)) {
		return func_ret_imm(v, s);
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
	if (vlen > asf_call_arg_regs_len)
		goto err_too_many_arg;
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	reg = asf_reg_get(asf_yz_type2bytes(type));
	*asf_regs[reg].purpose = ASF_REG_PURPOSE_FUNC_RESULT;
	if (asf_call_push_args(node->s, vlen, vs))
		goto err_free_node;
	if (asf_stack_top != NULL) {
		if (func_call_set_stack_top(node))
			goto err_free_node;
	}
	node_str_last = node->s->len;
	inst_len = strlen(temp) - 1 + strlen(name);
	str_expand(node->s, inst_len);
	snprintf(&node->s->s[node_str_last], inst_len, temp, name);
	return 0;
err_too_many_arg:
	printf("amc[backend.asf]: Too many arguments!\n");
	return 1;
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
	node->s = NULL;
	if (is_main) {
		if (func_ret_main(v, &node->s))
			goto err_free_node;
	} else if (func_ret_val(v, &node->s)) {
		goto err_free_node;
	}
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (node->s == NULL)
		node->s = str_new();
	str_append(node->s, strlen(temp), temp);
	return 0;
err_free_node:
	free(node);
	return 1;
}
