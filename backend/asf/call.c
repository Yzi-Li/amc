#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "../../include/backend/object.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"
#include "../../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int call_push_arg(str *s, yz_val *v, int index);
static int call_push_arg_arr(str *s, yz_array *arr, int index);
static int call_push_arg_expr(str *s, struct expr *expr, int index);
static int call_push_arg_identifier(str *s, struct symbol *sym, int index);
static int call_push_arg_imm(str *s, yz_val *v, int index);
static int call_push_arg_sym(str *s, struct symbol *sym, int index);

int call_push_arg(str *s, yz_val *v, int index)
{
	if (v->type == AMC_SYM) {
		return call_push_arg_sym(s, v->v, index);
	} else if (v->type == AMC_EXPR) {
		return call_push_arg_expr(s, v->v, index);
	} else if (v->type == YZ_ARRAY) {
		return call_push_arg_arr(s, v->v, index);
	} else if (YZ_IS_DIGIT(v->type)) {
		return call_push_arg_imm(s, v, index);
	}
	printf("amc[backend.asf]: call_push_arg: "
			"Unsupport argument type: \"%s\"\n",
			yz_get_type_name(v));
	return 1;
}

int call_push_arg_arr(str *s, yz_array *arr, int index)
{
	enum ASF_REGS dest = asf_call_arg_regs[index];
	str *tmp = NULL;
	if (arr->type.type != YZ_CHAR)
		return 1;
	dest += asf_reg_get(ASF_BYTES_U64);
	tmp = asf_inst_mov(ASF_MOV_C2R, &arr->type.i, &dest);
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int call_push_arg_expr(str *s, struct expr *expr, int index)
{
	enum ASF_REGS dest = asf_call_arg_regs[index],
	              src = ASF_REG_RAX;
	str *tmp = NULL;
	if (!YZ_IS_DIGIT(*expr->sum_type))
		return 1;
	src = asf_reg_get(asf_yz_type_raw2bytes(*expr->sum_type));
	dest += src;
	tmp = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int call_push_arg_identifier(str *s, struct symbol *sym, int index)
{
	str *tmp = NULL;
	enum ASF_REGS dest = asf_call_arg_regs[index];
	struct asf_stack_element *src = asf_identifier_get(sym->name);
	if (src == NULL)
		goto err_identifier_not_found;
	dest += asf_reg_get(src->bytes);
	tmp = asf_inst_mov(ASF_MOV_M2R, src, &dest);
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
err_identifier_not_found:
	printf("amc[backend.asf]: call_push_arg_identifier: "
			"\"%s\"\n", sym->name);
	return 1;
}

int call_push_arg_imm(str *s, yz_val *v, int index)
{
	struct asf_imm imm = {
		.type = asf_yz_type_raw2bytes(v->type),
		.iq = v->l
	};
	enum ASF_REGS reg = asf_call_arg_regs[index] + asf_reg_get(imm.type);
	str *tmp = asf_inst_mov(ASF_MOV_I2R, &imm, &reg);
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int call_push_arg_sym(str *s, struct symbol *sym, int index)
{
	enum ASF_REGS dest = asf_call_arg_regs[index],
	              src = ASF_REG_RAX;
	str *tmp = NULL;
	if (sym->args == NULL && sym->argc == 1)
		return call_push_arg_identifier(s, sym, index);
	src = asf_reg_get(asf_yz_type2bytes(&sym->result_type));
	dest += src;
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			return 0;
		src += asf_call_arg_regs[sym->argc - 2];
	}
	tmp = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int asf_call_push_args(int vlen, yz_val **vs)
{
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	for (int i = 0; i < vlen; i++) {
		if (i > asf_call_arg_regs_len)
			goto err_too_many_arg;
		if (vs[i] == NULL)
			goto err_arg_not_exists;
		if (call_push_arg(node->s, vs[i], i))
			return 1;
	}
	return 0;
err_free_node:
	free(node);
	return 1;
err_too_many_arg:
	printf("amc[backend.asf]: call_push_args: Too many arguments!\n");
	goto err_free_node;
err_arg_not_exists:
	printf("amc[backend.asf]: call_push_args: Argument is not exists!\n");
	goto err_free_node;
}

str *asf_inst_syscall(int code, int vlen, yz_val **vs)
{
	int str_last = 0;
	const char *temp =
		"movq $%d, %%rax\n"
		"syscall\n";
	str *s = str_new();
	if (asf_call_push_args(vlen, vs))
		goto err_free_str;
	str_last = s->len;
	str_expand(s, strlen(temp) - 2 + ullen(code));
	snprintf(&s->s[str_last], s->len, temp, code);
	return s;
err_free_str:
	str_free(s);
	return NULL;
}
