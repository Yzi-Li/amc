#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/stack.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"

static str *op_get_val_from_mem(struct object_node *parent,
		enum ASF_REGS dest);
static str *op_get_val_from_reg(struct object_node *parent, enum ASF_REGS src,
		enum ASF_REGS dest);
static str *op_get_val_imm(struct object_node *parent, yz_val *src,
		enum ASF_REGS dest);
static str *op_get_vall_from_mem_or_reg(struct object_node *parent,
		struct expr *e, enum ASF_REGS dest);
static str *op_get_vall_identifier(struct object_node *parent,
		struct symbol *src, enum ASF_REGS dest);
static str *op_get_vall_imm(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest);
static str *op_get_vall_sym(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest);
static str *op_get_valr_expr(struct object_node *parent, struct expr *src,
		enum ASF_REGS dest);
static str *op_get_valr_identifier(struct object_node *parent,
		struct symbol *src);
static str *op_get_valr_imm(struct object_node *parent, yz_val *src,
		enum ASF_REGS dest);
static str *op_get_valr_sym(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest);
static int op_init_obj_node(struct object_node *parent,
		struct object_node *node);

str *op_get_val_from_mem(struct object_node *parent, enum ASF_REGS dest)
{
	struct object_node *node = NULL;
	if (dest == -1)
		return asf_stack_get_element(asf_stack_top, 1);
	node = malloc(sizeof(*node));
	if (op_init_obj_node(parent, node))
		goto err_free_node;
	if ((node->s = asf_inst_pop(dest)) == NULL)
		goto err_inst_failed;
	return asf_reg_get_str(&asf_regs[dest]);
err_free_node:
	free(node);
	return NULL;
err_inst_failed:
	printf("amc[backend.asf:%s]: op_get_val_from_mem: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

str *op_get_val_from_reg(struct object_node *parent, enum ASF_REGS src,
		enum ASF_REGS dest)
{
	struct object_node *node = NULL;
	if (dest == -1)
		return asf_reg_get_str(&asf_regs[src]);
	if (src == dest)
		return asf_reg_get_str(&asf_regs[dest]);
	node = malloc(sizeof(*node));
	if (op_init_obj_node(parent, node))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_R2R, &src, &dest)) == NULL)
		goto err_inst_failed;
	return asf_reg_get_str(&asf_regs[dest]);
err_free_node:
	free(node);
	return NULL;
err_inst_failed:
	printf("amc[backend.asf:%s]: op_get_val_from_reg: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

str *op_get_val_imm(struct object_node *parent, yz_val *src,
		enum ASF_REGS dest)
{
	struct asf_imm imm = {
		.type = asf_yz_type2bytes(src),
		.iq = src->l
	};
	struct object_node *node = NULL;
	if (dest == -1)
		return asf_imm_str_new(&imm);
	node = malloc(sizeof(*node));
	if (op_init_obj_node(parent, node))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_I2R, &imm, &dest)) == NULL)
		goto err_inst_failed;
	return asf_reg_get_str(&asf_regs[dest]);
err_free_node:
	free(node);
	return NULL;
err_inst_failed:
	printf("amc[backend.asf:%s]: op_get_val_imm: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

str *op_get_vall_from_mem_or_reg(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest)
{
	enum ASF_REGS src = ASF_REG_RAX;
	if (e->valr->type == AMC_EXPR)
		return op_get_val_from_mem(parent, dest);
	src = asf_reg_get(asf_yz_type_raw2bytes(*e->sum_type));
	return op_get_val_from_reg(parent, src, dest);
}

str *op_get_vall_identifier(struct object_node *parent, struct symbol *src,
		enum ASF_REGS dest)
{
	char *name = str2chr(src->name, src->name_len);
	struct asf_stack_element *identifier = asf_identifier_get(name);
	struct object_node *node = NULL;
	if (identifier == NULL)
		goto err_identifier_not_found;
	free(name);
	node = malloc(sizeof(*node));
	if (op_init_obj_node(parent, node))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, identifier, &dest)) == NULL)
		goto err_inst_failed;
	return asf_reg_get_str(&asf_regs[dest]);
err_identifier_not_found:
	printf("amc[backend.asf:%s]: op_get_vall_identifier: "
			"Identifier not found: \"%s\"!\n", __FILE__, name);
	free(name);
	return NULL;
err_free_node:
	free(node);
	return NULL;
err_inst_failed:
	printf("amc[backend.asf:%s]: op_get_vall_identifier: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

str *op_get_vall_imm(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest)
{
	if (*asf_regs[dest].purpose != ASF_REG_PURPOSE_NULL
			&& !(e->valr->type == AMC_EXPR
				|| e->valr->type == AMC_SYM)) {
		if (asf_op_save_reg(parent, dest))
			return NULL;
	}
	*asf_regs[dest].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	return op_get_val_imm(parent, e->vall, dest);
}

str *op_get_vall_sym(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest)
{
	enum ASF_REGS reg = ASF_REG_RAX;
	struct symbol *src = e->vall->v;
	if (src->args == NULL && src->argc > 0
			&& *asf_regs[dest].purpose != ASF_REG_PURPOSE_NULL
			&& e->valr->type != AMC_EXPR
			&& e->valr->type != AMC_SYM) {
		if (asf_op_save_reg(parent, dest))
			return NULL;
	}
	*asf_regs[dest].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	if (src->args == NULL && src->argc == 1)
		return op_get_vall_identifier(parent, e->vall->v, dest);
	if (src->args == NULL && src->argc > 1) {
		if (src->argc - 2 > asf_call_arg_regs_len)
			return NULL;
		reg = asf_call_arg_regs[src->argc - 2]
			+ asf_reg_get(asf_yz_type_raw2bytes(*e->sum_type));
		return op_get_val_from_reg(parent, reg, dest);
	}
	return op_get_vall_from_mem_or_reg(parent, e, dest);
}

str *op_get_valr_expr(struct object_node *parent, struct expr *src,
		enum ASF_REGS dest)
{
	enum ASF_REGS reg = asf_reg_get(asf_yz_type_raw2bytes(*src->sum_type));
	if (dest == -1)
		dest = ASF_REG_RCX + reg;
	return op_get_val_from_reg(parent, reg, dest);
}

str *op_get_valr_identifier(struct object_node *parent, struct symbol *src)
{
	char *name = str2chr(src->name, src->name_len);
	struct asf_stack_element *identifier = asf_identifier_get(name);
	if (identifier == NULL)
		goto err_identifier_not_found;
	free(name);
	return asf_stack_get_element(identifier, 0);
err_identifier_not_found:
	printf("amc[backend.asf:%s]: op_get_valr_identifier: "
			"Identifier not found: \"%s\"!\n", __FILE__, name);
	free(name);
	return NULL;
}

str *op_get_valr_imm(struct object_node *parent, yz_val *src,
		enum ASF_REGS dest)
{
	struct object_node *node = NULL;
	if (dest != -1 && *asf_regs[dest].purpose != ASF_REG_PURPOSE_NULL) {
		if (asf_op_save_reg(parent, dest))
			return NULL;
		node = malloc(sizeof(*node));
		if (parent == NULL) {
			if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
				goto err_free_node;
		} else {
			if (object_insert(node, parent, parent->next))
				goto err_free_node;
		}
		if ((node->s = asf_inst_pop(dest)) == NULL)
			goto err_inst_failed;
	}
	return op_get_val_imm(parent, src, dest);
err_free_node:
	free(node);
	return NULL;
err_inst_failed:
	printf("amc[backend.asf:%s]: op_get_valr_imm: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

str *op_get_valr_sym(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest)
{
	struct symbol *src = e->valr->v;
	enum ASF_REGS reg = asf_reg_get(asf_yz_type2bytes(&src->result_type));
	if (src->args == NULL && src->argc == 1)
		return op_get_valr_identifier(parent, src);
	if (src->args == NULL && src->argc > 1) {
		if (src->argc - 2 > asf_call_arg_regs_len)
			return NULL;
		reg += asf_call_arg_regs[src->argc - 2];
		return op_get_val_from_reg(parent, reg, dest);
	}
	if (e->vall->type != AMC_SYM)
		return op_get_val_from_mem(parent, dest);
	if (dest == -1)
		dest = ASF_REG_RCX + reg;
	return op_get_val_from_reg(parent, reg, dest);
}

int op_init_obj_node(struct object_node *parent, struct object_node *node)
{
	if (node == NULL)
		return 0;
	if (parent == NULL) {
		if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
			return 1;
		return 0;
	}
	if (object_insert(node, parent->prev, parent))
		return 1;
	return 0;
}

int asf_op_clean_reg(struct object_node *parent, enum ASF_REGS reg)
{
	struct object_node *node = malloc(sizeof(*node));
	if (op_init_obj_node(parent, node))
		goto err_free_node;
	if ((node->s = asf_reg_clean(reg)) == NULL)
		goto err_inst_failed;
	return 0;
err_free_node:
	free(node);
	return 1;
err_inst_failed:
	printf("amc[backend.asf:%s]: asf_op_clean_reg: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

str *asf_op_get_val_left(struct object_node *parent, struct expr *e)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	dest += asf_reg_get(asf_yz_type_raw2bytes(*e->sum_type));
	if (e->vall->type == AMC_EXPR) {
		*asf_regs[dest].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
		return op_get_vall_from_mem_or_reg(parent, e, dest);
	} else if (e->vall->type == AMC_SYM) {
		return op_get_vall_sym(parent, e, dest);
	} else if (YZ_IS_DIGIT(e->vall->type)) {
		return op_get_vall_imm(parent, e, dest);
	}
	return NULL;
}

str *asf_op_get_val_right(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest)
{
	if (dest != -1 && dest <= ASF_REG_RSP)
		dest += asf_reg_get(asf_yz_type_raw2bytes(*e->sum_type));
	if (e->valr->type == AMC_EXPR) {
		return op_get_valr_expr(parent, e, dest);
	} else if (e->valr->type == AMC_SYM) {
		return op_get_valr_sym(parent, e, dest);
	} else if (YZ_IS_DIGIT(e->valr->type)) {
		return op_get_valr_imm(parent, e->valr, dest);
	}
	return NULL;
}

int asf_op_save_reg(struct object_node *parent, enum ASF_REGS reg)
{
	struct object_node *node = malloc(sizeof(*node));
	if (op_init_obj_node(parent, node))
		goto err_free_node;
	if ((node->s = asf_inst_push_reg(reg)) == NULL)
		goto err_inst_failed;
	return 0;
err_free_node:
	free(node);
	return 1;
err_inst_failed:
	printf("amc[backend.asf:%s]: asf_op_save_reg: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}
