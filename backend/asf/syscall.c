#include "inst.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"
#include "../../utils/utils.h"
#include <stdio.h>

static enum ASF_REGS call_arg_regs[] = {
	ASF_REG_RDI,
	ASF_REG_RSI,
	ASF_REG_RDX,
	ASF_REG_RCX,
	ASF_REG_R8,
	ASF_REG_R9
};
static const int call_arg_regs_len = LENGTH(call_arg_regs);

static int syscall_push_arg(str *s, yz_val *v, int index);
static int syscall_push_arg_expr(str *s, struct expr *expr, int index);
static int syscall_push_arg_imm(str *s, yz_val *v, int index);
static int syscall_push_arg_sym(str *s, struct symbol *sym, int index);
static int syscall_push_args(str *s, int vlen, yz_val **vs);

int syscall_push_arg(str *s, yz_val *v, int index)
{
	if (v->type == AMC_SYM) {
		return syscall_push_arg_sym(s, v->v, index);
	} else if (v->type == AMC_EXPR) {
		return syscall_push_arg_expr(s, v->v, index);
	} else if (YZ_IS_DIGIT(v->type)) {
		return syscall_push_arg_imm(s, v, index);
	}
	printf("amc[backend.asf]: syscall_push_arg: "
			"Unsupport argument type: \"%s\"\n",
			yz_get_type_name(v->type));
	return 1;
}

int syscall_push_arg_expr(str *s, struct expr *expr, int index)
{
	enum ASF_REGS dest = call_arg_regs[index],
	              src = ASF_REG_RAX;
	str *tmp = NULL;
	if (!YZ_IS_DIGIT(*expr->sum_type))
		return 1;
	src = asf_reg_get(asf_yz_type2imm(*expr->sum_type));
	dest += src;
	tmp = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int syscall_push_arg_imm(str *s, yz_val *v, int index)
{
	struct asf_imm imm = {
		.type = asf_yz_type2imm(v->type),
		.iq = v->l
	};
	enum ASF_REGS reg = call_arg_regs[index] + asf_reg_get(imm.type);
	str *tmp = asf_inst_mov(ASF_MOV_I2R, &imm, &reg);
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int syscall_push_arg_sym(str *s, struct symbol *sym, int index)
{
	enum ASF_REGS dest = call_arg_regs[index],
	              src = ASF_REG_RAX;
	str *tmp = NULL;
	if (sym->args == NULL && sym->argc == 1) {
		printf("amc[backend.asf]: syscall_push_arg: "
				"Unsupport syntax: identifier.\n");
		return 1;
	}
	src = asf_reg_get(asf_yz_type2imm(sym->result_type));
	dest += src;
	tmp = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	str_append(s, tmp->len - 1, tmp->s);
	str_free(tmp);
	return 0;
}

int syscall_push_args(str *s, int vlen, yz_val **vs)
{
	for (int i = 0; i < vlen; i++) {
		if (i > call_arg_regs_len)
			goto err_too_many_arg;
		if (vs[i] == NULL)
			goto err_arg_not_exists;
		if (syscall_push_arg(s, vs[i], i))
			return 1;
	}
	return 0;
err_too_many_arg:
	printf("amc[backend.asf]: syscall_push_args: Too many arguments!\n");
	return 1;
err_arg_not_exists:
	printf("amc[backend.asf]: syscall_push_args: "
			"Argument is not exists!\n");
	return 1;
}

str *asf_inst_syscall(int code, int vlen, yz_val **vs)
{
	int str_last = 0;
	const char *temp =
		"movq $%d, %%rax\n"
		"syscall\n";
	str *s = str_new();
	if (syscall_push_args(s, vlen, vs))
		goto err_free_str;
	str_last = s->len;
	str_expand(s, strlen(temp) - 2 + ullen(code));
	snprintf(&s->s[str_last], s->len, temp, code);
	return s;
err_free_str:
	str_free(s);
	return NULL;
}
