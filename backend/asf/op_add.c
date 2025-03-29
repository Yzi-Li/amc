#include "asf.h"
#include "imm.h"
#include "inst.h"
#include "op.h"
#include "register.h"
#include "suffix.h"

static const char *temp = "add%c %s, %s\n";

static int add_expr_and_expr(struct object_node *node, struct expr *e);

int add_expr_and_expr(struct object_node *node, struct expr *e)
{
	enum ASF_REGS addend_reg = ASF_REG_RDX;
	str *addend_reg_str = NULL,
	    *augend_str = NULL;
	addend_reg = asf_reg_get(asf_yz_type2imm(
				*((struct expr *)e->valr->v)->sum_type));
	addend_reg_str = asf_reg_get_str(&asf_regs[addend_reg]);
	augend_str = asf_stack_get_element(asf_stack_top, 1);
	str_expand(node->s, strlen(temp) - 4
			+ augend_str->len - 1
			+ addend_reg_str->len - 1);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_regs[addend_reg].size),
			augend_str->s,
			addend_reg_str->s);
	str_free(addend_reg_str);
	str_free(augend_str);
	return 0;
}

int asf_op_add(struct expr *e)
{
	str *addend = NULL,
	    *augend_str = NULL;
	enum ASF_REGS augend_reg = ASF_REG_RAX;
	struct asf_imm imm = {};
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (e->vall->type == AMC_EXPR && e->valr->type == AMC_EXPR)
		return add_expr_and_expr(node, e);
	if (asf_op_try_save_val(node, e->vall, &augend_reg))
		goto err_free_node;
	imm.type = asf_yz_type2imm(e->valr->type);
	imm.iq = e->valr->l;
	addend = asf_imm_str_new(&imm);
	str_expand(node->s, (strlen(temp) - 1)
			+ addend->len);
	augend_str = asf_reg_get_str(&asf_regs[augend_reg]);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_regs[augend_reg].size),
			addend->s,
			augend_str->s);
	str_free(addend);
	str_free(augend_str);
	asf_regs[augend_reg].flags.used = 1;
	*asf_regs[augend_reg].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
