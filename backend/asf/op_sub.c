#include "asf.h"
#include "inst.h"
#include "imm.h"
#include "op.h"
#include "register.h"
#include "suffix.h"

static const char *temp = "sub%c %s, %s\n";

static int sub_expr_and_expr(struct object_node *node, struct expr *e);

int sub_expr_and_expr(struct object_node *node, struct expr *e)
{
	enum ASF_REGS minuend_reg = ASF_REG_RAX,
	              subtrahend_reg = ASF_REG_RDX;
	str *get_minuend = NULL,
	    *get_subtrahend = NULL,
	    *minuend_str = NULL,
	    *subtrahend_str = NULL;
	int inst_sub_len = 0,
	    prefix_len = 0;
	subtrahend_reg += asf_reg_get(asf_yz_type2imm(*e->sum_type));
	get_minuend = asf_inst_pop(&minuend_reg);
	get_subtrahend = asf_inst_mov(ASF_MOV_R2R,
			&minuend_reg, &subtrahend_reg);
	minuend_str = asf_reg_get_str(&asf_regs[minuend_reg]);
	subtrahend_str = asf_reg_get_str(&asf_regs[subtrahend_reg]);
	inst_sub_len = strlen(temp) - 2
			+ minuend_str->len - 1
			+ subtrahend_str->len - 1;
	str_append(node->s, get_subtrahend->len - 1, get_subtrahend->s);
	str_append(node->s, get_minuend->len - 1, get_minuend->s);
	prefix_len = node->s->len;
	str_expand(node->s, inst_sub_len);
	snprintf(&node->s->s[prefix_len], inst_sub_len, temp,
			asf_suffix_get(asf_regs[subtrahend_reg].size),
			subtrahend_str->s,
			minuend_str->s);
	str_free(get_minuend);
	str_free(get_subtrahend);
	str_free(minuend_str);
	str_free(subtrahend_str);
	return 0;
}

int asf_op_sub(struct expr *e)
{
	struct asf_imm imm = {};
	struct object_node *node = NULL;
	enum ASF_REGS minuend_reg = ASF_REG_RAX,
	              subtrahend_reg = ASF_REG_RDX;
	str *minuend_str = NULL,
	    *subtrahend_str = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(objs[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (e->vall->type == AMC_EXPR && e->valr->type == AMC_EXPR)
		return sub_expr_and_expr(node, e);
	if (e->valr->type == AMC_EXPR) {
		minuend_reg = ASF_REG_RDX;
		if (asf_op_try_save_val(node, e->vall, &minuend_reg))
			goto err_free_node;
		minuend_str = asf_reg_get_str(&asf_regs[minuend_reg]);
		subtrahend_reg = asf_reg_get(asf_yz_type2imm(
				*((struct expr*)e->valr->v)->sum_type));
		subtrahend_str = asf_reg_get_str(&asf_regs[subtrahend_reg]);
	} else {
		if (asf_op_try_save_val(node, e->vall, &minuend_reg))
			goto err_free_node;
		minuend_str = asf_reg_get_str(&asf_regs[minuend_reg]);
		imm.type = asf_yz_type2imm(e->valr->type);
		imm.iq = e->valr->l;
		subtrahend_str = asf_imm_str_new(&imm);
	}
	str_expand(node->s, strlen(temp) - 2
			+ subtrahend_str->len - 1
			+ minuend_str->len - 1);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_regs[minuend_reg].size),
			subtrahend_str->s,
			minuend_str->s);
	if (e->valr->type == AMC_EXPR) {
		node->s->len -= 1;
		str_free(minuend_str);
		minuend_str = asf_inst_mov(ASF_MOV_R2R,
				&minuend_reg, &subtrahend_reg);
		str_append(node->s, minuend_str->len, minuend_str->s);
	}
	str_free(minuend_str);
	str_free(subtrahend_str);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
