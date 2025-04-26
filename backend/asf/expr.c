#include "include/asf.h"
#include "include/imm.h"
#include "include/register.h"
#include "include/suffix.h"
#include "../../include/type.h"
#include "../../include/backend/target.h"

static const char *op_inst_cmp = "cmp%c %s, %s\n";
int asf_op_cmp(struct object_node *node, yz_val *l, yz_val *r)
{
	str *imm_s = NULL;
	str *reg_s = NULL;
	struct asf_imm imm = {};
	if (l->type == AMC_EXPR && r->type == AMC_EXPR) {
		return 0;
	} else if (l->type == AMC_EXPR) {
		imm.type = asf_yz_type2imm(r->type);
		imm.iq = r->l;
		imm_s = asf_imm_str_new(&imm);
		return 0;
	} else if (r->type == AMC_EXPR) {
		imm.type = asf_yz_type2imm(l->type);
		imm.iq = l->l;
		imm_s = asf_imm_str_new(&imm);
		return 0;
	} else {
		return 1;
	}
	reg_s = asf_reg_get_str(&asf_regs[ASF_REG_RAX]);
	str_expand(node->s, (strlen(op_inst_cmp) - 1)
			+ imm_s->len
			+ reg_s->len);
	snprintf(node->s->s, node->s->len, op_inst_cmp,
			asf_suffix_get(asf_regs[ASF_REG_RAX].size),
			imm_s->s,
			reg_s->s);
	return 0;
}

int asf_op_and(struct expr *e)
{
	return 0;
}

int asf_op_not(struct expr *e)
{
	return 0;
}

int asf_op_or(struct expr *e)
{
	return 0;
}
