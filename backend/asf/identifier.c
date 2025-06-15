#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/register.h"
#include "include/stack.h"
#include "../../include/symbol.h"
#include "../../include/backend/object.h"
#include <stdlib.h>
#include <string.h>


static int identifier_change_get_op_inst(struct object_node *parent,
		enum ASF_REGS src, int is_unsigned, enum OP_ID mode);
static int identifier_change_save_reg(struct object_node *parent,
		enum ASF_REGS src, enum ASF_REGS *dest);
static int identifier_change_val_from_reg(struct asf_stack_element *dest,
		enum OP_ID mode, enum ASF_REGS src, int is_unsigned);
static int identifier_change_val_from_mem(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_stack_element *src);
static int identifier_change_val_from_imm(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_imm *imm);
static int identifier_get_val(enum ASF_MOV_TYPE mode, enum ASF_REGS dest,
		void *src);
static str *identifier_set_arr(struct asf_stack_element *dest, enum OP_ID mode,
		yz_array *arr);
static str *identifier_set_expr(struct asf_stack_element *dest,
		enum OP_ID mode, struct expr *src);
static str *identifier_set_identifier(struct asf_stack_element *dest,
		enum OP_ID mode, struct symbol *src);
static str *identifier_set_imm(struct asf_stack_element *dest, enum OP_ID mode,
		yz_val *src);
static str *identifier_set_null(struct asf_stack_element *dest,
		enum OP_ID mode, yz_val *src);
static str *identifier_set_sym(struct asf_stack_element *dest, enum OP_ID mode,
		struct symbol *src);
static str *identifier_set(struct asf_stack_element *dest, enum OP_ID mode,
		yz_val *src);

int identifier_change_get_op_inst(struct object_node *parent,
		enum ASF_REGS src, int is_unsigned, enum OP_ID mode)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	if (object_insert(node, parent->prev, parent))
		goto err_free_node;
	dest = asf_reg_get(asf_regs[src].bytes);
	switch (mode) {
	case OP_ASSIGN_ADD:
		if ((node->s = asf_inst_op_add(src, dest)) == NULL)
			goto err_inst_failed;
		break;
	case OP_ASSIGN_DIV:
		if ((node->s = asf_inst_op_div(src, is_unsigned)) == NULL)
			goto err_inst_failed;
		break;
	case OP_ASSIGN_MUL:
		if ((node->s = asf_inst_op_mul(src, is_unsigned)) == NULL)
			goto err_inst_failed;
		break;
	case OP_ASSIGN_SUB:
		if ((node->s = asf_inst_op_sub(src, dest)) == NULL)
			goto err_inst_failed;
		break;
	default:
		return 1;
	}
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: change_val_from_reg: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int identifier_change_save_reg(struct object_node *parent, enum ASF_REGS src,
		enum ASF_REGS *dest)
{
	struct object_node *node = NULL;
	*dest = ASF_REG_RBX + asf_reg_get(asf_regs[src].bytes);
	node = malloc(sizeof(*node));
	if (object_insert(node, parent->prev, parent))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_R2R, &src, dest)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: change_val_from_reg: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int identifier_change_val_from_reg(struct asf_stack_element *dest,
		enum OP_ID mode, enum ASF_REGS src, int is_unsigned)
{
	enum ASF_REGS tmp = ASF_REG_RAX;
	struct object_node *node = NULL;
	tmp = asf_reg_get(asf_regs[src].bytes);
	node = malloc(sizeof(*node));
	if (object_insert(node, objs[cur_obj][ASF_OBJ_TEXT].last->prev,
				objs[cur_obj][ASF_OBJ_TEXT].last))
		goto err_free_node;
	if (*asf_regs[tmp].purpose != ASF_REG_PURPOSE_NULL || src == tmp)
		if (identifier_change_save_reg(node, tmp, &src))
			return 1;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, dest, &tmp)) == NULL)
		goto err_inst_failed;
	return identifier_change_get_op_inst(node->next, src, is_unsigned,
			mode);
err_inst_failed:
	printf("amc[backend.asf]: change_val_from_reg: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int identifier_change_val_from_mem(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_stack_element *src)
{
	enum ASF_REGS base = ASF_REG_RAX, tmp = ASF_REG_RBX;
	base = asf_reg_get(src->bytes);
	dest += base;
	tmp += base;
	if (identifier_get_val(ASF_MOV_M2R, tmp, src))
		return 1;
	return identifier_change_val_from_reg(dest, mode, tmp,
			src->bytes > ASF_BYTES_U_OFFSET);
}

int identifier_change_val_from_imm(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_imm *imm)
{
	enum ASF_REGS tmp = ASF_REG_RBX;
	tmp += asf_reg_get(imm->type);
	if (identifier_get_val(ASF_MOV_I2R, tmp, imm))
		return 1;
	return identifier_change_val_from_reg(dest, mode, tmp,
			imm->type > ASF_BYTES_U_OFFSET);
}

int identifier_get_val(enum ASF_MOV_TYPE mode, enum ASF_REGS dest, void *src)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_insert(node, objs[cur_obj][ASF_OBJ_TEXT].last->prev,
				objs[cur_obj][ASF_OBJ_TEXT].last))
		goto err_free_node;
	if ((node->s = asf_inst_mov(mode, src, &dest)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: change_val_from_reg: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

str *identifier_set_arr(struct asf_stack_element *dest, enum OP_ID mode,
		yz_array *arr)
{
	if (arr->type.type != YZ_CHAR)
		return NULL;
	return asf_inst_mov(ASF_MOV_C2M, &arr->type.i, dest);
}

str *identifier_set_expr(struct asf_stack_element *dest, enum OP_ID mode,
		struct expr *expr)
{
	enum ASF_BYTES bytes = asf_yz_type_raw2bytes(*expr->sum_type);
	enum ASF_REGS src = asf_reg_get(bytes);
	if (mode != OP_ASSIGN)
		if (identifier_change_val_from_reg(dest, mode, src,
					bytes > ASF_BYTES_U_OFFSET))
			return NULL;
	return asf_inst_mov(ASF_MOV_R2M, &src, dest);
}

str *identifier_set_identifier(struct asf_stack_element *dest, enum OP_ID mode,
		struct symbol *src)
{
	enum ASF_REGS src_reg = ASF_REG_RAX;
	if (mode != OP_ASSIGN) {
		src_reg = asf_reg_get(dest->bytes);
		if (identifier_change_val_from_mem(dest, mode,
					src->backend_status))
			return NULL;
		return asf_inst_mov(ASF_MOV_R2M, &src_reg, dest);
	}
	return asf_inst_mov(ASF_MOV_M2M, src->backend_status, dest);
}

str *identifier_set_imm(struct asf_stack_element *dest, enum OP_ID mode,
		yz_val *val)
{
	struct asf_imm imm = {
		.type = asf_yz_type_raw2bytes(val->type),
		.iq = val->l
	};
	enum ASF_REGS src = ASF_REG_RAX;
	if (mode == OP_ASSIGN)
		return asf_inst_mov(ASF_MOV_I2M, &imm, dest);
	if (identifier_change_val_from_imm(dest, mode, &imm))
		return NULL;
	src = asf_reg_get(imm.type);
	return asf_inst_mov(ASF_MOV_R2M, &src, dest);
}

str *identifier_set_null(struct asf_stack_element *dest, enum OP_ID mode,
		yz_val *val)
{
	struct asf_imm imm = {
		.type = ASF_BYTES_U64,
		.iq = 0
	};
	if (mode != OP_ASSIGN)
		return NULL;
	return asf_inst_mov(ASF_MOV_I2M, &imm, dest);
}

str *identifier_set_sym(struct asf_stack_element *dest, enum OP_ID mode,
		struct symbol *sym)
{
	enum ASF_BYTES bytes = asf_yz_type2bytes(&sym->result_type);
	enum ASF_REGS src = asf_reg_get(bytes);
	if (sym->args == NULL && sym->argc == 1)
		return identifier_set_identifier(dest, mode, sym);
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			return NULL;
		src += asf_call_arg_regs[sym->argc - 2];
	}
	if (mode != OP_ASSIGN) {
		if (identifier_change_val_from_reg(dest, mode, src,
					bytes > ASF_BYTES_U_OFFSET))
			return NULL;
		src = asf_reg_get(asf_regs[src].bytes);
	}
	return asf_inst_mov(ASF_MOV_R2M, &src, dest);
}

str *identifier_set(struct asf_stack_element *dest, enum OP_ID mode,
		yz_val *src)
{
	if (src->type == AMC_EXPR) {
		return identifier_set_expr(dest, mode, src->v);
	} else if (src->type == AMC_SYM) {
		return identifier_set_sym(dest, mode, src->v);
	} else if (src->type == YZ_NULL) {
		return identifier_set_null(dest, mode, src);
	} else if (src->type == YZ_ARRAY) {
		return identifier_set_arr(dest, mode, src->v);
	} else if (YZ_IS_DIGIT(src->type)) {
		return identifier_set_imm(dest, mode, src);
	}
	printf("amc[backend.asf:%s]: identifier_set: Unsupport type: \"%s\"\n",
			__FILE__, yz_get_type_name(src));
	return NULL;
}

int asf_var_set(backend_symbol_status **raw_sym_stat,
		enum OP_ID mode, yz_val *val)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (mode == OP_ASSIGN && *raw_sym_stat == NULL) {
		if ((node->s = asf_inst_push(val)) == NULL)
			goto err_inst_failed;
		*raw_sym_stat = asf_stack_top;
		return 0;
	}
	if ((node->s = identifier_set(*raw_sym_stat, mode, val)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_var_set: Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int asf_var_immut_init(backend_symbol_status **raw_sym_stat, yz_val *val)
{
	return asf_var_set(raw_sym_stat, OP_ASSIGN, val);
}

int asf_identifier_reg(backend_symbol_status **raw_sym_stat,
		struct asf_stack_element *src)
{
	if (*raw_sym_stat != NULL)
		goto err_registered;
	*raw_sym_stat = src;
	return 0;
err_registered:
	printf("amc[backend.asf]: asf_identifier_reg: "
			"Identifier registered!\n");
	return 1;
}
