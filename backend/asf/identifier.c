#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/register.h"
#include "include/scope.h"
#include "include/stack.h"
#include "../../include/symbol.h"
#include "../../include/backend/object.h"
#include <stdlib.h>
#include <string.h>

typedef int sym_id_t;

struct defined_sym {
	char *index;
	struct asf_stack_element *stack;
};

static struct defined_sym *defined_syms = NULL;
static int defined_sym_len = 0;
static int defined_sym_capacity = 0;

static int change_get_op_inst(struct object_node *parent, enum ASF_REGS src,
		int is_unsigned, enum OP_ID mode);
static int change_save_reg(struct object_node *parent, enum ASF_REGS src,
		enum ASF_REGS *dest);
static int change_val_from_reg(sym_id_t id, enum ASF_REGS src, int is_unsigned,
		enum OP_ID mode);
static int change_val_from_mem(sym_id_t id, struct asf_stack_element *src,
		enum OP_ID mode);
static int change_val_from_imm(sym_id_t id, struct asf_imm *imm,
		enum OP_ID mode);
static sym_id_t get_id(char *name);
static sym_id_t reg_id(char *name, backend_scope_status *raw_status);
static str *set_expr(sym_id_t id, struct expr *expr, enum OP_ID mode);
static str *set_identifier(sym_id_t id, struct symbol *sym, enum OP_ID mode);
static str *set_imm(sym_id_t id, yz_val *val, enum OP_ID mode);
static str *set_null(sym_id_t id, yz_val *val, enum OP_ID mode);
static str *set_sym(sym_id_t id, struct symbol *sym, enum OP_ID mode);
static str *set_val(sym_id_t id, yz_val *val, enum OP_ID mode);

int change_get_op_inst(struct object_node *parent, enum ASF_REGS src,
		int is_unsigned, enum OP_ID mode)
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

int change_save_reg(struct object_node *parent, enum ASF_REGS src,
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

int change_val_from_reg(sym_id_t id, enum ASF_REGS src, int is_unsigned,
		enum OP_ID mode)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	struct object_node *node = NULL;
	dest = asf_reg_get(asf_regs[src].bytes);
	node = malloc(sizeof(*node));
	if (object_insert(node, objs[cur_obj][ASF_OBJ_TEXT].last->prev,
				objs[cur_obj][ASF_OBJ_TEXT].last))
		goto err_free_node;
	if (*asf_regs[dest].purpose != ASF_REG_PURPOSE_NULL || src == dest)
		if (change_save_reg(node, dest, &src))
			return 1;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, defined_syms[id].stack,
					&dest)) == NULL)
		goto err_inst_failed;
	return change_get_op_inst(node->next, src, is_unsigned, mode);
err_inst_failed:
	printf("amc[backend.asf]: change_val_from_reg: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int change_val_from_mem(sym_id_t id, struct asf_stack_element *src,
		enum OP_ID mode)
{
	enum ASF_REGS dest = ASF_REG_RBX;
	struct object_node *node = NULL;
	dest += asf_reg_get(src->bytes);
	node = malloc(sizeof(*node));
	if (object_insert(node, objs[cur_obj][ASF_OBJ_TEXT].last->prev,
				objs[cur_obj][ASF_OBJ_TEXT].last))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, src, &dest)) == NULL)
		goto err_inst_failed;
	return change_val_from_reg(id, dest, src->bytes > ASF_BYTES_U_OFFSET,
			mode);
err_inst_failed:
	printf("amc[backend.asf]: change_val_from_reg: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

int change_val_from_imm(sym_id_t id, struct asf_imm *imm, enum OP_ID mode)
{
	enum ASF_REGS dest = ASF_REG_RBX;
	struct object_node *node = NULL;
	dest += asf_reg_get(imm->type);
	node = malloc(sizeof(*node));
	if (object_insert(node, objs[cur_obj][ASF_OBJ_TEXT].last->prev,
				objs[cur_obj][ASF_OBJ_TEXT].last))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_I2R, imm, &dest)) == NULL)
		goto err_inst_failed;
	return change_val_from_reg(id, dest,
			imm->type > ASF_BYTES_U_OFFSET, mode);
err_inst_failed:
	printf("amc[backend.asf]: change_val_from_reg: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
}

sym_id_t get_id(char *name)
{
	if (name == NULL)
		return -1;
	for (int i = 0; i < defined_sym_len; i++) {
		if (strcmp(name, defined_syms[i].index) == 0)
			return i;
	}
	return -1;
}

sym_id_t reg_id(char *name, backend_scope_status *raw_status)
{
	struct asf_scope_status *status = raw_status;
	char *name_copy = NULL;
	if (name == NULL)
		return -1;
	name_copy = malloc(strlen(name) + 1);
	memcpy(name_copy, name, strlen(name) + 1);
	if (defined_syms == NULL) {
		defined_sym_capacity = 1;
		defined_sym_len = 1;
		defined_syms = malloc(sizeof(*defined_syms));
		defined_syms[0].index = name_copy;
		status->identifier_count += 1;
		return 0;
	}
	defined_sym_len += 1;
	if (defined_sym_len > defined_sym_capacity) {
		defined_sym_capacity += 1;
		defined_syms = realloc(defined_syms,
				defined_sym_capacity * sizeof(*defined_syms));
	}
	defined_syms[defined_sym_len - 1].index = name_copy;
	status->identifier_count += 1;
	return defined_sym_len - 1;
}

str *set_expr(sym_id_t id, struct expr *expr, enum OP_ID mode)
{
	enum ASF_BYTES bytes = asf_yz_type_raw2bytes(*expr->sum_type);
	enum ASF_REGS src = asf_reg_get(bytes);
	if (mode != OP_ASSIGN)
		if (change_val_from_reg(id, src, bytes > ASF_BYTES_U_OFFSET,
					mode))
			return NULL;
	return asf_inst_mov(ASF_MOV_R2M, &src, defined_syms[id].stack);
}

str *set_identifier(sym_id_t id, struct symbol *sym, enum OP_ID mode)
{
	struct asf_stack_element *src = NULL;
	sym_id_t src_id = -1;
	char *src_name = NULL;
	enum ASF_REGS src_reg = ASF_REG_RAX;
	src_name = str2chr(sym->name, sym->name_len);
	if ((src_id = get_id(src_name)) == -1)
		goto err_id_not_exists;
	free(src_name);
	src = defined_syms[src_id].stack;
	if (mode != OP_ASSIGN) {
		src_reg = asf_reg_get(defined_syms[id].stack->bytes);
		if (change_val_from_mem(id, src, mode))
			return NULL;
		return asf_inst_mov(ASF_MOV_R2M, &src_reg,
				defined_syms[id].stack);
	}
	return asf_inst_mov(ASF_MOV_M2M, src, defined_syms[id].stack);
err_id_not_exists:
	printf("amc[backend.asf:%s]: set_identifier: "
			"Identifier not exists!\n",
			__FILE__);
	free(src_name);
	return NULL;
}

str *set_imm(sym_id_t id, yz_val *val, enum OP_ID mode)
{
	struct asf_imm imm = {
		.type = asf_yz_type_raw2bytes(val->type),
		.iq = val->l
	};
	enum ASF_REGS src = ASF_REG_RAX;
	if (mode == OP_ASSIGN)
		return asf_inst_mov(ASF_MOV_I2M, &imm, defined_syms[id].stack);
	if (change_val_from_imm(id, &imm, mode))
		return NULL;
	src = asf_reg_get(imm.type);
	return asf_inst_mov(ASF_MOV_R2M, &src, defined_syms[id].stack);
}

str *set_null(sym_id_t id, yz_val *val, enum OP_ID mode)
{
	struct asf_imm imm = {
		.type = ASF_BYTES_U64,
		.iq = 0
	};
	if (mode != OP_ASSIGN)
		return NULL;
	return asf_inst_mov(ASF_MOV_I2M, &imm, defined_syms[id].stack);
}

str *set_sym(sym_id_t id, struct symbol *sym, enum OP_ID mode)
{
	enum ASF_BYTES bytes = asf_yz_type2bytes(&sym->result_type);
	enum ASF_REGS src = asf_reg_get(bytes);
	if (sym->args == NULL && sym->argc == 1)
		return set_identifier(id, sym, mode);
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			return NULL;
		src += asf_call_arg_regs[sym->argc - 2];
	}
	if (mode != OP_ASSIGN) {
		if (change_val_from_reg(id, src, bytes > ASF_BYTES_U_OFFSET,
					mode))
			return NULL;
		src = asf_reg_get(asf_regs[src].bytes);
	}
	return asf_inst_mov(ASF_MOV_R2M, &src, defined_syms[id].stack);
}

str *set_val(sym_id_t id, yz_val *val, enum OP_ID mode)
{
	if (val->type == AMC_EXPR) {
		return set_expr(id, val->v, mode);
	} else if (val->type == AMC_SYM) {
		return set_sym(id, val->v, mode);
	} else if (val->type == YZ_NULL) {
		return set_null(id, val, mode);
	} else if (YZ_IS_DIGIT(val->type)) {
		return set_imm(id, val, mode);
	}
	printf("amc[backend.asf:%s]: set_val: Unsupport type: \"%s\"\n",
			__FILE__,
			yz_get_type_name(val));
	return NULL;
}

int asf_var_set(char *name, yz_val *val, enum OP_ID mode,
		backend_scope_status *raw_status)
{
	sym_id_t id = -1;
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((id = get_id(name)) == -1) {
		if ((id = reg_id(name, raw_status)) == -1)
			goto err_free_node;
		if ((node->s = asf_inst_push(val)) == NULL)
			goto err_inst_failed;
		defined_syms[id].stack = asf_stack_top;
		return 0;
	}
	if ((node->s = set_val(id, val, mode)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_var_set: Get instruction failed!\n");
	goto err_free_node;
err_free_node:
	free(node);
	return 1;
}

int asf_var_immut_init(char *name, yz_val *val,
		backend_scope_status *raw_status)
{
	return asf_var_set(name, val, OP_ASSIGN, raw_status);
}

void asf_identifier_free_id(int num)
{
	for (int i = 0; i < num; i++) {
		defined_sym_len--;
		if (defined_syms[defined_sym_len].index != NULL)
			free(defined_syms[defined_sym_len].index);
		defined_syms[defined_sym_len].index = NULL;
	}
}

struct asf_stack_element *asf_identifier_get(char *name)
{
	sym_id_t id = -1;
	if ((id = get_id(name)) == -1)
		return NULL;
	return defined_syms[id].stack;
}

int asf_identifier_reg(char *name, struct asf_stack_element *src,
		backend_scope_status *raw_status)
{
	sym_id_t id = -1;
	if (name == NULL || src == NULL)
		return 1;
	if ((id = get_id(name)) != -1)
		goto err_registered;
	if ((id = reg_id(name, raw_status)) == -1)
		return 1;
	defined_syms[id].stack = src;
	return 0;
err_registered:
	printf("amc[backend.asf]: asf_identifier_reg: "
			"Identifier registered!\n");
	return 1;
}
