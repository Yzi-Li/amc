#include "call.h"
#include "identifier.h"
#include "imm.h"
#include "inst.h"
#include "op.h"
#include "stack.h"
#include "../../include/expr.h"
#include "../../include/symbol.h"
#include "../../include/token.h"

static str *get_val_expr(struct object_node *parent, struct expr *e);
static str *get_val_identifier(struct symbol *sym);
static str *get_val_sym(struct object_node *parent, struct symbol *sym);
static int save_identifier(struct object_node *parent, struct symbol *sym,
		enum ASF_REGS *dest);
static int save_sym(struct object_node *parent, struct symbol *sym,
		enum ASF_REGS *dest);

str *get_val_expr(struct object_node *parent, struct expr *e)
{
	enum ASF_REGS dest = ASF_REG_RDX,
	              src = ASF_REG_RAX;
	struct object_node *node = calloc(1, sizeof(*node));
	if (object_insert(node, parent->prev, parent))
		goto err_free_node;
	src = asf_reg_get(asf_yz_type2imm(*e->sum_type));
	dest += src;
	node->s = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	return asf_reg_get_str(&asf_regs[dest]);
err_free_node:
	str_free(node->s);
	free(node);
	return NULL;
}

str *get_val_identifier(struct symbol *sym)
{
	char *name = tok2str(sym->name, sym->name_len);
	struct asf_stack_element *src = asf_identifier_get(name);
	if (src == NULL)
		goto err_free_name;
	free(name);
	return asf_stack_get_element(src, 0);
err_free_name:
	free(name);
	return NULL;
}

str *get_val_sym(struct object_node *parent, struct symbol *sym)
{
	enum ASF_REGS dest = ASF_REG_RDX,
	              src = ASF_REG_RAX;
	struct object_node *node = NULL;
	if (sym->args == NULL && sym->argc == 1)
		return get_val_identifier(sym);
	src = asf_reg_get(asf_yz_type2imm(sym->result_type));
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			goto err_free_node;
		src += asf_call_arg_regs[sym->argc - 2];
		return asf_reg_get_str(&asf_regs[src]);
	}
	node = calloc(1, sizeof(*node));
	if (object_insert(node, parent->prev, parent))
		goto err_free_node;
	dest += src;
	node->s = asf_inst_mov(ASF_MOV_R2R, &src, &dest);
	return asf_reg_get_str(&asf_regs[dest]);
err_free_node:
	str_free(node->s);
	free_safe(node);
	return NULL;
}

int save_identifier(struct object_node *parent, struct symbol *sym,
		enum ASF_REGS *dest)
{
	char *name = tok2str(sym->name, sym->name_len);
	struct object_node *node = malloc(sizeof(*node));
	struct asf_stack_element *src = asf_identifier_get(name);
	if (src == NULL)
		goto err_free_name;
	if (object_insert(node, parent->prev, parent))
		goto err_free_all;
	*dest += asf_reg_get(asf_yz_type2imm(sym->result_type));
	node->s = asf_inst_mov(ASF_MOV_M2R, src, dest);
	free(name);
	return 0;
err_free_all:
	str_free(node->s);
	free(node);
err_free_name:
	free(name);
	return 1;
}

int save_sym(struct object_node *parent, struct symbol *sym,
		enum ASF_REGS *dest)
{
	struct object_node *node = NULL;
	enum ASF_REGS src = ASF_REG_RAX;
	if (sym->args == NULL && sym->argc == 1)
		return save_identifier(parent, sym, dest);
	src = asf_reg_get(asf_yz_type2imm(sym->result_type));
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			goto err_free_node;
		*dest = src;
		src += asf_call_arg_regs[sym->argc - 2];
	} else {
		*dest += src;
	}
	if (*dest == src)
		return 0;
	node = malloc(sizeof(*node));
	if (object_insert(node, parent->prev, parent))
		goto err_free_node;
	node->s = asf_inst_mov(ASF_MOV_R2R, &src, dest);
	return 0;
err_free_node:
	str_free(node->s);
	free_safe(node);
	return 1;
}

int asf_op_clean_reg(struct object_node *parent, enum ASF_REGS reg)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_insert(node, parent->prev, parent))
		goto err_free_node;
	node->s = asf_reg_clean(reg);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

str *asf_op_get_val(struct object_node *parent, yz_val *src)
{
	struct asf_imm imm = {};
	if (src->type == AMC_EXPR) {
		return get_val_expr(parent, src->v);
	} else if (src->type == AMC_SYM) {
		return get_val_sym(parent, src->v);
	} else if (YZ_IS_DIGIT(src->type)) {
		imm.type = asf_yz_type2imm(src->type);
		imm.iq = src->l;
		return asf_imm_str_new(&imm);
	}
	return NULL;
}

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
	} else if (src->type == AMC_SYM) {
		return save_sym(parent, src->v, dest);
	}
	*dest = *dest + asf_reg_get(asf_yz_type2imm(src->type));
	if (asf_regs[*dest].flags.used)
		if (asf_op_save_reg(parent, *dest))
			return 1;
	if (asf_op_save_val(parent, src, *dest))
		return 1;
	return 0;
}
