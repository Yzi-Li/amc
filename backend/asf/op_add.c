#include "asf.h"
#include "imm.h"
#include "op.h"
#include "suffix.h"

static const char *temp = "add%c %s, %s\n";

int asf_op_add(yz_val *l, yz_val *r)
{
	str *addend = NULL;
	struct asf_imm imm = {};
	struct object_node *node = NULL;
	enum ASF_REGS result_reg = ASF_REG_RAX;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node)) {
		str_free(node->s);
		free(node);
		return 1;
	}
	if (l->type == AMC_SUB_EXPR && r->type == AMC_SUB_EXPR) {
		str_append(node->s, 32,
				"addq (%rsp), %rax\n"
				"addq $8, %rsp\n");
		return 0;
	}
	if (regs[result_reg].flags.used)
		asf_op_save_reg(node, &regs[result_reg]);
	if (l->type != AMC_SUB_EXPR)
		asf_op_save_val(node, l, result_reg);
	imm.type = asf_yz_type2imm(r->type);
	imm.iq = r->l;
	addend = asf_imm_str_new(&imm);
	str_expand(node->s, (strlen(temp) - 1)
			+ addend->len);
	snprintf(node->s->s, node->s->len, temp,
			suffix_get(regs[result_reg].size),
			addend->s,
			asf_reg_get_chr(&regs[result_reg]));
	str_free(addend);
	regs[result_reg].flags.used = 1;
	regs[result_reg].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	return 0;
}
