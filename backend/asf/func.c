#include "asf.h"
#include "imm.h"
#include "inst.h"
#include "register.h"
#include "../../include/backend/target.h"
#include "../../utils/utils.h"

static const enum ASF_REGS asf_func_call_arg_regs[] = {
	ASF_REG_RDI,
	ASF_REG_RSI,
	ASF_REG_RDX,
	ASF_REG_RCX,
	ASF_REG_R8,
	ASF_REG_R9
};
static const int asf_func_call_arg_regs_len = LENGTH(asf_func_call_arg_regs);

static int asf_func_call_basic_args(struct object_node *node,
		yz_val **v, int vlen);
static int asf_func_call_ext_args(struct object_node *node,
		yz_val **v, int vlen);
static str *asf_func_ret_imm(yz_val *v);
static int asf_func_ret_val(yz_val *v, str **s);

int asf_func_call_basic_args(struct object_node *node,
		yz_val **v, int vlen)
{
	str *tmp = NULL;
	for (int i = 0; i < asf_func_call_arg_regs_len; i++) {
		if (i > vlen)
			return 1;
		tmp = asf_inst_mov(ASF_MOV_I2R, v[i],
				&regs[asf_func_call_arg_regs[i]]);
		str_append(node->s, tmp->len, tmp->s);
		str_free(tmp);
	}
	return 0;
}

int asf_func_call_ext_args(struct object_node *node,
		yz_val **v, int vlen)
{
	str *tmp = NULL;
	struct asf_imm imm = {.type = ASF_IMM_NULL, .iq = 0};
	for (int i = 0; i < vlen; i++) {
		imm.type = asf_yz_type2imm(v[i]->type);
		imm.iq = v[i]->l;
		tmp = asf_inst_pushi(&imm);
		str_append(node->s, tmp->len, tmp->s);
		str_free(tmp);
	}
	return 0;
}

str *asf_func_ret_imm(yz_val *v)
{
	struct asf_imm imm = {};
	enum ASF_REGS result_reg = ASF_REG_RAX;
	imm.type = asf_yz_type2imm(v->type);
	imm.iq = v->l;
	if (REGION_INT(imm.type, ASF_IMM8, ASF_IMM16)) {
		imm.type = ASF_IMM32;
	} else if (REGION_INT(imm.type, ASF_IMMU8, ASF_IMMU16)) {
		imm.type = ASF_IMMU32;
	}
	if (imm.type == ASF_IMM32)
		result_reg = ASF_REG_EAX;
	return asf_inst_mov(ASF_MOV_I2R, &imm, &result_reg);
}

int asf_func_ret_val(yz_val *v, str **s)
{
	if (regs[ASF_REG_RAX].purpose == ASF_REG_PURPOSE_EXPR_RESULT) {
		regs[ASF_REG_RAX].purpose = ASF_REG_PURPOSE_FUNC_RESULT;
		return -1;
	} else if (regs[ASF_REG_EAX].purpose == ASF_REG_PURPOSE_EXPR_RESULT) {
		regs[ASF_REG_EAX].purpose = ASF_REG_PURPOSE_FUNC_RESULT;
		return -1;
	} if (REGION_INT(v->type, YZ_I8, YZ_U64)) {
		//TODO purpose set
		*s = asf_func_ret_imm(v);
		return 0;
	}
	return 1;
}

int asf_func_call(const char *name, yz_val **v, int vlen)
{
	struct object_node *node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(objs[ASF_OBJ_TEXT], node))
		goto err;
	if (asf_func_call_basic_args(node, v, vlen))
		asf_func_call_ext_args(node, v, vlen);
	return 0;
err:
	str_free(node->s);
	free(node);
	return 1;
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
		goto err;
	memcpy(tmp_name, name, len);
	tmp_name[len] = '\0';
	str_expand(node->s, (strlen(temp) - 4) + (len * 2));
	snprintf(node->s->s, node->s->len, temp, tmp_name, tmp_name);
	free(tmp_name);
	return 0;
err:
	str_free(node->s);
	free(node);
	free(tmp_name);
	return 1;
}

int asf_func_ret(yz_val *v)
{
	const char *temp =
		"popq %rbp\n"
		"ret\n";
	int ret = 0;
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(objs[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((ret = asf_func_ret_val(v, &node->s)) > 0)
		goto err_free_node;
	if (ret == -1)
		node->s = str_new();
	str_append(node->s, strlen(temp), temp);
	return 0;
err_free_node:
	free(node);
	return 1;
}
