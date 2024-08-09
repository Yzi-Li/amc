#include "asf.h"
#include "op.h"

// TODO: signed support
int asf_op_div(yz_val *l, yz_val *r)
{
	str *tmp = NULL;
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(objs[ASF_OBJ_TEXT], node)) {
		str_free(node->s);
		free(node);
		return 1;
	}
	if (regs[ASF_REG_RDX].flags.used)
		asf_op_save_reg(node, &regs[ASF_REG_RDX]);
	tmp = asf_reg_clean(&regs[ASF_REG_RDX]);
	str_append(node->s, tmp->len, tmp->s);
	str_free(tmp);
	if (l->type == AMC_SUB_EXPR && r->type == AMC_SUB_EXPR) {
		str_append(node->s, 26,
				"divq (%rsp)\n"
				"addq $8, %rsp\n");
		return 0;
	}
	return 0;
}
