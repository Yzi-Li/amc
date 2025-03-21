#include "asf.h"
#include "imm.h"
#include "inst.h"
#include "register.h"
#include "../../include/backend/target.h"
#include "../../utils/utils.h"

static enum ASF_REGS asf_func_call_arg_regs[] = {
	ASF_REG_RDI,
	ASF_REG_RSI,
	ASF_REG_RDX,
	ASF_REG_RCX,
	ASF_REG_R8,
	ASF_REG_R9
};
static const int asf_func_call_arg_regs_len = LENGTH(asf_func_call_arg_regs);

static int asf_func_ret_imm(yz_val *v, enum ASF_REGS reg);
static int asf_func_ret_val(yz_val *v);

int asf_func_ret_imm(yz_val *v, enum ASF_REGS reg)
{
	struct object_node *node = malloc(sizeof(*node));
	struct asf_imm imm = {};
	imm.type = asf_yz_type2imm(v->type);
	imm.iq = v->l;
	if (REGION_INT(imm.type, ASF_IMM8, ASF_IMM16)) {
		imm.type = ASF_IMM32;
	} else if (REGION_INT(imm.type, ASF_IMMU8, ASF_IMMU16)) {
		imm.type = ASF_IMMU32;
	}
	if (object_append(objs[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = asf_inst_mov(ASF_MOV_I2R, &imm, &reg);
	asf_regs[reg].purpose = ASF_REG_PURPOSE_FUNC_RESULT;
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_func_ret_val(yz_val *v)
{
	enum ASF_IMM_TYPE imm_type;
	enum ASF_REGS reg = ASF_REG_RAX;
	if (v->type == AMC_SYM)
		return 0;
	imm_type = asf_yz_type2imm(v->type);
	if (REGION_INT(imm_type, ASF_IMM8, ASF_IMM16)) {
		imm_type = ASF_IMM32;
	} else if (REGION_INT(imm_type, ASF_IMMU8, ASF_IMMU16)) {
		imm_type = ASF_IMMU32;
	}
	reg = asf_reg_get(imm_type);
	if (asf_regs[reg].purpose == ASF_REG_PURPOSE_EXPR_RESULT) {
		asf_regs[reg].purpose = ASF_REG_PURPOSE_FUNC_RESULT;
		return 0;
	} else if (REGION_INT(v->type, YZ_I8, YZ_U64)) {
		return asf_func_ret_imm(v, reg);
	}
	return 1;
}

int asf_func_call(const char *name, yz_val **v, int vlen)
{
	struct object_node *node = malloc(sizeof(*node));
	struct asf_imm imm = {};
	str *tmp = NULL;
	node->s = str_new();
	object_append(objs[ASF_OBJ_TEXT], node);
	for (int i = 0; i < vlen; i++) {
		imm.type = asf_yz_type2imm(v[i]->type);
		imm.iq = v[i]->l;
		if (i < asf_func_call_arg_regs_len) {
			tmp = asf_inst_mov(ASF_MOV_I2R, &imm,
					&asf_func_call_arg_regs[i]);
		} else {
			tmp = asf_inst_pushi(&imm, ASF_STACK_MODE_NATIVE);
		}
		str_append(node->s, tmp->len, tmp->s);
		tmp->len = 0;
		free_safe(tmp->s);
	}
	return 0;
}

int asf_func_def(const char *name, int len, enum YZ_TYPE type)
{
	const char *temp =
		".globl %s\n"
		"%s:\n"
		"pushq %%rbp\n";
	char *tmp_name = malloc(len + 1);
	struct object_node *node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(objs[ASF_OBJ_TEXT], node))
		goto err_free_node;
	memcpy(tmp_name, name, len);
	tmp_name[len] = '\0';
	str_expand(node->s, (strlen(temp) - 4) + (len * 2));
	snprintf(node->s->s, node->s->len, temp, tmp_name, tmp_name);
	free(tmp_name);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_func_ret(yz_val *v)
{
	const char *temp =
		"popq %rbp\n"
		"ret\n";
	int ret = 0;
	struct object_node *node = malloc(sizeof(*node));
	if ((ret = asf_func_ret_val(v)) > 0)
		goto err_free_node;
	if (object_append(objs[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	str_append(node->s, strlen(temp), temp);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
