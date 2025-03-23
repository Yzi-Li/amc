#include "asf.h"
#include "imm.h"
#include "register.h"
#include "suffix.h"
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

int asf_op_eq(struct expr *e)
{
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err;
	if (asf_op_cmp(node, e->vall, e->valr))
		goto err;
	return 0;
err:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_op_ge(struct expr *e)
{
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err;
	if (asf_op_cmp(node, e->vall, e->valr))
		goto err;
	return 0;
err:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_op_gt(struct expr *e)
{
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err;
	if (asf_op_cmp(node, e->vall, e->valr))
		goto err;
	return 0;
err:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_op_le(struct expr *e)
{
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err;
	if (asf_op_cmp(node, e->vall, e->valr))
		goto err;
	return 0;
err:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_op_lt(struct expr *e)
{
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err;
	if (asf_op_cmp(node, e->vall, e->valr))
		goto err;
	return 0;
err:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_op_ne(struct expr *e)
{
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err;
	if (asf_op_cmp(node, e->vall, e->valr))
		goto err;
	return 0;
err:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_op_not(struct expr *e)
{
	return 0;
}

int asf_op_or(struct expr *e)
{
	return 0;
}

int asf_op_assignment(struct expr *e)
{
	return 0;
}

int asf_var_set(str *name, yz_val *val)
{
	return 0;
}

int asf_var_immut_set(str *name, yz_val *val)
{
	return 0;
}
