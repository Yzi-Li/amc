#include "asf.h"
#include "imm.h"
#include "inst.h"
#include "op.h"

int asf_op_save_reg(struct object_node *root, struct asf_reg *reg)
{
	struct object_node *node = malloc(sizeof(*node));
	if (root == NULL) {
		if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node)) {
			free(node);
			return 1;
		}
	} else {
		if (object_insert(node, root->prev, root)) {
			free(node);
			return 1;
		}
	}
	node->s = asf_inst_push(reg->size, asf_reg_get_chr(&regs[ASF_REG_RAX]));
	reg->purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	return 0;
}

int asf_op_save_val(struct object_node *root, yz_val *v, enum ASF_REGS r)
{
	struct asf_imm imm = {};
	struct object_node *node = malloc(sizeof(*node));
	if (root == NULL) {
		if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node)) {
			free(node);
			return 1;
		}
	} else {
		if (object_insert(node, root->prev, root)) {
			free(node);
			return 1;
		}
	}
	imm.type = asf_yz_type2imm(v->type);
	imm.iq = v->l;
	node->s = asf_inst_mov(ASF_MOV_I2R, &imm, &r);
	regs[r].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	return 0;
}
