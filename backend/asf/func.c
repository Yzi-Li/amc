#include "asf.h"
#include "imm.h"
#include "inst.h"
#include "register.h"
#include "../../include/backend/target.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"
#include "../../utils/utils.h"

static enum ASF_REGS func_call_arg_regs[] = {
	ASF_REG_RDI,
	ASF_REG_RSI,
	ASF_REG_RDX,
	ASF_REG_RCX,
	ASF_REG_R8,
	ASF_REG_R9
};
static const int func_call_arg_regs_len = LENGTH(func_call_arg_regs);

static int func_call_basic_args(str *s, yz_val **vs, int vlen);
static int func_call_ext_args(str *s, yz_val **vs, int vlen);
static int func_call_push_arg(str *s, int index, yz_val *v);
static int func_call_push_arg_expr(str *s, int index, struct expr *expr);
static int func_call_push_arg_imm(str *s, int index, yz_val *v);
static int func_call_push_arg_sym(str *s, int index, struct symbol *sym);
static void func_ret_clean_stack();
static int func_ret_expr(struct expr *expr);
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
	} else if (REGION_INT(v->type, YZ_I8, YZ_U64)) {
		return func_call_push_arg_imm(s, index, v);
	}
	printf("amc[backend.asf]: func_call_push_arg: "
			"Unsupport argument type: \"%s\"\n",
			yz_get_type_name(v->type));
	return 1;
}

int func_call_push_arg_expr(str *s, int index, struct expr *expr)
{
	enum ASF_REGS dest = ASF_REG_RDI,
	              src = ASF_REG_RAX;
	str *src_str = NULL, *tmp = NULL;
	src = asf_reg_get(asf_yz_type2imm(*expr->sum_type));
	if (index > func_call_arg_regs_len) {
		src_str = asf_reg_get_str(&asf_regs[src]);
		if (src_str->s[src_str->len - 1] != '\0')
			str_append(src_str, 1, "\0");
		tmp = asf_inst_push(asf_regs[src].size,
				src_str->s, ASF_STACK_MODE_LOCAL);
		str_free(src_str);
	} else {
		dest = src + func_call_arg_regs[index];
		tmp = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	}
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int func_call_push_arg_imm(str *s, int index, yz_val *v)
{
	struct asf_imm imm = {
		.type = asf_yz_type2imm(v->type),
		.iq = v->l
	};
	str *tmp = NULL;
	if (index > func_call_arg_regs_len) {
		tmp = asf_inst_pushi(&imm, ASF_STACK_MODE_NATIVE);
	} else {
		tmp = asf_inst_mov(ASF_MOV_I2R, &imm,
				&func_call_arg_regs[index]);
	}
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int func_call_push_arg_sym(str *s, int index, struct symbol *sym)
{
	str *src_str = NULL, *tmp = NULL;
	enum ASF_REGS dest = ASF_REG_RDI,
	              src = ASF_REG_RAX;
	if (sym->args == NULL && sym->argc == 1) {
		printf("amc[backend.asf]: func_call_push_arg_sym: "
				"Unsupport syntax.\n");
		return 1;
	}
	src = asf_reg_get(asf_yz_type2imm(sym->result_type));
	if (index > func_call_arg_regs_len) {
		if (src_str->s[src_str->len - 1] != '\0')
			str_append(src_str, 1, "\0");
		tmp = asf_inst_push(asf_regs[src].size,
				src_str->s, ASF_STACK_MODE_LOCAL);
		str_free(src_str);
	} else {
		dest = src + func_call_arg_regs[index];
		tmp = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	}
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
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
	return;
}

int func_ret_expr(struct expr *expr)
{
	enum ASF_REGS reg = asf_reg_get(asf_yz_type2imm(*expr->sum_type));
	if (*asf_regs[reg].purpose != ASF_REG_PURPOSE_EXPR_RESULT)
		return 1;
	*asf_regs[reg].purpose = ASF_REG_PURPOSE_NULL;
	asf_regs[reg].flags.used = 0;
	return 0;
}

int func_ret_imm(yz_val *v)
{
	struct asf_imm imm = {};
	struct object_node *node = malloc(sizeof(*node));
	enum ASF_REGS reg = ASF_REG_RAX;
	imm.type = asf_yz_type2imm(v->type);
	imm.iq = v->l;
	if (REGION_INT(imm.type, ASF_IMM8, ASF_IMM16)) {
		imm.type = ASF_IMM32;
	} else if (REGION_INT(imm.type, ASF_IMMU8, ASF_IMMU16)) {
		imm.type = ASF_IMMU32;
	}
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
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
	struct object_node *node = malloc(sizeof(*node));
	enum ASF_REGS src = ASF_REG_RAX,
	              dest = ASF_REG_RDI;
	src = asf_reg_get(asf_yz_type2imm(sym->result_type));
	dest += src;
	node->s = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
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
	} else if (REGION_INT(v->type, YZ_I8, YZ_U64)) {
		return func_ret_imm(v);
	}
	return 1;
}

int asf_func_call(const char *name, yz_val **vs, int vlen)
{
	struct object_node *node = malloc(sizeof(*node));
	int node_str_last = 0;
	int inst_len = 0;
	const char *temp = "call %s\n";
	node->s = str_new();
	object_append(&objs[cur_obj][ASF_OBJ_TEXT], node);
	if (func_call_basic_args(node->s, vs, vlen))
		return 1;
	if (vlen > func_call_arg_regs_len) {
		if (func_call_ext_args(node->s,
				&vs[func_call_arg_regs_len],
				vlen - func_call_arg_regs_len))
			return 1;
	}
	node_str_last = node->s->len;
	inst_len = strlen(temp) + strlen(name);
	str_expand(node->s, inst_len);
	snprintf(&node->s->s[node_str_last], inst_len, temp, name);
	return 0;
}

int asf_func_def(const char *name, int len, enum YZ_TYPE type)
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
