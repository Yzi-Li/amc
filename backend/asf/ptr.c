/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/mov.h"
#include "include/stack.h"
#include "include/val.h"
#include "../../include/backend/object.h"

enum GET_ADDR_RESULT {
	GET_ADDR_DONE,
	GET_ADDR_DONE_AND_REG_PUSHED,
	GET_ADDR_FAULT
};

static enum GET_ADDR_RESULT
ptr_set_val_get_addr(struct symbol *ident, enum ASF_REGS addr);

static str *ptr_set_val_imm(struct asf_mem *dest, enum OP_ID mode,
		struct asf_imm *src);
static str *ptr_set_val_mem(struct asf_mem *dest, enum OP_ID mode,
		struct asf_mem *src);
static str *ptr_set_val_reg(struct asf_mem *dest, enum OP_ID mode,
		enum ASF_REGS src);
static int ptr_set_val_restore_addr_reg(enum ASF_REGS reg);

enum GET_ADDR_RESULT
ptr_set_val_get_addr(struct symbol *ident, enum ASF_REGS addr)
{
	struct asf_mem mem = {};
	struct object_node *node;
	int reg_pushed = 0;
	str *tmp = NULL;
	if (ident->type != SYM_IDENTIFIER)
		goto err_unsupport_sym;
	node = malloc(sizeof(*node));
	asf_stack_element2mem(ident->backend_status, &mem);
	if (*asf_regs[addr].purpose != ASF_REG_PURPOSE_NULL) {
		if ((node->s = asf_inst_push_reg(addr)) == NULL)
			goto err_free_node;
		reg_pushed = 1;
	}
	if ((tmp = asf_inst_mov_m2r(&mem, addr)) == NULL)
		goto err_free_node;
	if (reg_pushed) {
		str_append(node->s, tmp->len, tmp->s);
		str_free(tmp);
	} else {
		node->s = tmp;
	}
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return reg_pushed ? GET_ADDR_DONE_AND_REG_PUSHED : GET_ADDR_DONE;
err_unsupport_sym:
	printf("amc[backend.asf:%s]: ptr_set_val_get_dest: "
			"Unsupport symbol type\n",
			__FILE__);
	return GET_ADDR_FAULT;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return GET_ADDR_FAULT;
}

str *ptr_set_val_imm(struct asf_mem *dest, enum OP_ID mode,
		struct asf_imm *src)
{
	return asf_inst_mov_i2m(src, dest);
}

str *ptr_set_val_mem(struct asf_mem *dest, enum OP_ID mode,
		struct asf_mem *src)
{
	return asf_inst_mov_m2m(src, dest);
}

str *ptr_set_val_reg(struct asf_mem *dest, enum OP_ID mode, enum ASF_REGS src)
{
	return asf_inst_mov_r2m(src, dest);
}

int ptr_set_val_restore_addr_reg(enum ASF_REGS reg)
{
	struct object_node *node = malloc(sizeof(*node));
	if ((node->s = asf_inst_pop(reg)) == NULL)
		goto err_free_node;
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return 1;
}

int asf_ptr_set_val(struct symbol *ident, yz_val *val, enum OP_ID mode)
{
	enum ASF_REGS addr = ASF_REG_RBX;
	struct asf_mem dest = {
		.addr = addr,
		.offset = 0
	};
	enum GET_ADDR_RESULT get_addr_result = GET_ADDR_FAULT;
	struct object_node *node = NULL;
	struct asf_val v = {};
	if (asf_val_get(val, &v))
		goto err_unsupport_type;
	if (ident->result_type.type != YZ_PTR)
		return 1;
	if ((get_addr_result = ptr_set_val_get_addr(ident, addr))
			== GET_ADDR_FAULT)
		return 1;
	node = malloc(sizeof(*node));
	switch (v.type) {
	case ASF_VAL_IMM:
		dest.bytes = v.imm.type;
		if ((node->s = ptr_set_val_imm(&dest, mode, &v.imm)) == NULL)
			goto err_free_node;
		break;
	case ASF_VAL_MEM:
		dest.bytes = v.mem.bytes;
		if ((node->s = ptr_set_val_mem(&dest, mode, &v.mem)) == NULL)
			goto err_free_node;
		break;
	case ASF_VAL_REG:
		dest.bytes = asf_regs[v.reg].bytes;
		if ((node->s = ptr_set_val_reg(&dest, mode, v.reg)) == NULL)
			goto err_free_node;
		break;
	default: goto err_unsupport_type; break;
	}
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	if (get_addr_result == GET_ADDR_DONE_AND_REG_PUSHED
			&& ptr_set_val_restore_addr_reg(addr))
		return 1;
	return 0;
err_unsupport_type:
	printf("amc[backend.asf:%s]: asf_ptr_set_val: "
			"Unsupport type: \"%s\"\n",
			__FILE__, yz_get_type_name(&val->type));
	return 1;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return 1;
}
