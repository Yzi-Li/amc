#include "imm.h"
#include "inst.h"
#include "op.h"
#include "../../include/expr.h"

int asf_op_save_reg(struct object_node *parent, enum ASF_REGS reg)
{
	struct object_node *node = malloc(sizeof(*node));
	str *reg_str = NULL;
	if (object_insert(node, parent->prev, parent))
		goto err_free_node;
	reg_str = asf_reg_get_str(&asf_regs[reg]);
	node->s = asf_inst_push_reg(reg);
	str_free(reg_str);
	return 0;
err_free_node:
	free(node);
	return 1;
}

int asf_op_save_val(struct object_node *parent, yz_val *v, enum ASF_REGS r)
{
	struct asf_imm imm = {};
	struct object_node *node = malloc(sizeof(*node));
	if (object_insert(node, parent->prev, parent))
		goto err_free_node;
	imm.type = asf_yz_type2imm(v->type);
	imm.iq = v->l;
	node->s = asf_inst_mov(ASF_MOV_I2R, &imm, &r);
	*asf_regs[r].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	asf_regs[r].flags.used = 1;
	return 0;
err_free_node:
	free(node);
	return 1;
}

int asf_op_try_save_val(struct object_node *parent, yz_val *src,
		enum ASF_REGS *dest)
{
	if (dest == NULL)
		return 1;
	if (*dest > ASF_REG_RSP)
		return 1;
	if (src->type == AMC_EXPR) {
		*dest = *dest + asf_reg_get(asf_yz_type2imm(
					*((struct expr*)src->v)->sum_type));
		return 0;
	}
	*dest = *dest + asf_reg_get(asf_yz_type2imm(src->type));
	if (asf_regs[*dest].flags.used)
		if (asf_op_save_reg(parent, *dest))
			return 1;
	if (asf_op_save_val(parent, src, *dest))
		return 1;
	return 0;
}
