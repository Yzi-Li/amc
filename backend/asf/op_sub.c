#include "asf.h"
#include "imm.h"
#include "op.h"
#include "suffix.h"

static const char *temp = "sub%c %s, %s\n";

int asf_op_sub(yz_val *l, yz_val *r)
{
	struct asf_imm imm = {};
	struct object_node *node = NULL;
	str *subtrahend = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(objs[ASF_OBJ_TEXT], node)) {
		str_free(node->s);
		free(node);
		return 1;
	}
	imm.type = asf_yz_type2imm(r->type);
	imm.iq = r->l;
	if (l->type == AMC_SUB_EXPR && r->type == AMC_SUB_EXPR) {
		str_append(node->s, 32,
				"subq (%rsp), %rax\n"
				"addq $8, %rsp\n");
		return 0;
	}
	subtrahend = asf_imm_str_new(&imm);
	str_expand(node->s, (strlen(temp) - 2)
			+ subtrahend->len);
	snprintf(node->s->s, node->s->len, temp,
			suffix_get(regs[ASF_REG_RAX].size),
			subtrahend->s,
			regs[ASF_REG_RAX].name);
	str_free(subtrahend);
	return 0;
}
