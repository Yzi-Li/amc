#include "include/asf.h"
#include "include/identifier.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/val.h"
#include "../../include/backend/object.h"
#include "../../include/symbol.h"
#include <stdlib.h>
#include <string.h>

static int identifier_change_get_op_inst(struct object_node *parent,
		enum ASF_REGS src, int is_unsigned, enum OP_ID mode);
static int identifier_change_val_from_imm(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_imm *imm);
static int identifier_change_val_from_mem(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_stack_element *src);
static int identifier_change_val_from_reg(struct asf_stack_element *dest,
		enum OP_ID mode, enum ASF_REGS src, int is_unsigned);
static int identifier_get_val(enum ASF_MOV_TYPE mode, void *src,
		enum ASF_REGS dest);
static str *identifier_set_const(struct asf_stack_element *dest,
		enum OP_ID mode, int src);
static str *identifier_set_imm(struct asf_stack_element *dest, enum OP_ID mode,
		struct asf_imm *src);
static str *identifier_set_mem(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_stack_element *src);
static str *identifier_set_reg(struct asf_stack_element *dest,
		enum OP_ID mode, enum ASF_REGS src);

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

int identifier_change_val_from_imm(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_imm *imm)
{
	enum ASF_REGS tmp = ASF_REG_RBX;
	tmp += asf_reg_get(imm->type);
	if (identifier_get_val(ASF_MOV_I2R, imm, tmp))
		return 1;
	return identifier_change_val_from_reg(dest, mode, tmp,
			imm->type > ASF_BYTES_U_OFFSET);
}

int identifier_change_val_from_mem(struct asf_stack_element *dest,
		enum OP_ID mode, struct asf_stack_element *src)
{
	enum ASF_REGS base = ASF_REG_RAX, tmp = ASF_REG_RBX;
	base = asf_reg_get(src->bytes);
	dest += base;
	tmp += base;
	if (identifier_get_val(ASF_MOV_M2R, src, tmp))
		return 1;
	return identifier_change_val_from_reg(dest, mode, tmp,
			src->bytes > ASF_BYTES_U_OFFSET);
}

int identifier_change_val_from_reg(struct asf_stack_element *dest,
		enum OP_ID mode, enum ASF_REGS src, int is_unsigned)
{
	enum ASF_REGS tmp = ASF_REG_RAX;
	tmp = asf_reg_get(asf_regs[src].bytes);
	if (identifier_get_val(ASF_MOV_M2R, dest, tmp))
		return 1;
	return identifier_change_get_op_inst(objs[cur_obj][ASF_OBJ_TEXT].last,
			src, is_unsigned, mode);
}

int identifier_get_val(enum ASF_MOV_TYPE mode, void *src, enum ASF_REGS dest)
{
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
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

str *identifier_set_const(struct asf_stack_element *dest, enum OP_ID mode,
		int src)
{
	return asf_inst_mov(ASF_MOV_C2M, &src, dest);
}

str *identifier_set_imm(struct asf_stack_element *dest, enum OP_ID mode,
		struct asf_imm *src)
{
	enum ASF_REGS reg = ASF_REG_RAX;
	if (mode == OP_ASSIGN)
		return asf_inst_mov(ASF_MOV_I2M, src, dest);
	if (identifier_change_val_from_imm(dest, mode, src))
		return NULL;
	reg = asf_reg_get(src->type);
	return asf_inst_mov(ASF_MOV_R2M, &reg, dest);
}

str *identifier_set_mem(struct asf_stack_element *dest, enum OP_ID mode,
		struct asf_stack_element *src)
{
	enum ASF_REGS src_reg = ASF_REG_RAX;
	if (mode != OP_ASSIGN) {
		src_reg = asf_reg_get(dest->bytes);
		if (identifier_change_val_from_mem(dest, mode, src))
			return NULL;
		return asf_inst_mov(ASF_MOV_R2M, &src_reg, dest);
	}
	return asf_inst_mov(ASF_MOV_M2M, src, dest);
}

str *identifier_set_reg(struct asf_stack_element *dest, enum OP_ID mode,
		enum ASF_REGS src)
{
	if (mode != OP_ASSIGN) {
		if (identifier_change_val_from_reg(dest, mode, src, 0))
			return NULL;
	}
	return asf_inst_mov(ASF_MOV_R2M, &src, dest);
}

int asf_var_set(struct symbol *sym, enum OP_ID mode, yz_val *val)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (mode == OP_ASSIGN && sym->backend_status == NULL) {
		if ((node->s = asf_inst_push(val)) == NULL)
			goto err_inst_failed;
		sym->backend_status = asf_stack_top;
		return 0;
	}
	if ((node->s = asf_identifier_set(sym->backend_status, mode, val)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_var_set: Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int asf_var_immut_init(struct symbol *sym, yz_val *val)
{
	return asf_var_set(sym, OP_ASSIGN, val);
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

str *asf_identifier_set(struct asf_stack_element *dest, enum OP_ID mode,
		yz_val *src)
{
	struct asf_val val = {};
	if (asf_val_get(src, &val))
		goto err_unsupport_type;
	if (val.type == ASF_VAL_CONST) {
		return identifier_set_const(dest, mode, val.const_id);
	} else if (val.type == ASF_VAL_IMM) {
		return identifier_set_imm(dest, mode, &val.imm);
	} else if (val.type == ASF_VAL_MEM) {
		return identifier_set_mem(dest, mode, val.mem);
	} else if (val.type == ASF_VAL_REG) {
		return identifier_set_reg(dest, mode, val.reg);
	}
err_unsupport_type:
	printf("amc[backend.asf:%s]: identifier_set: Unsupport type: \"%s\"\n",
			__FILE__, yz_get_type_name(src));
	return NULL;
}
