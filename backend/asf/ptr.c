/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/mov.h"
#include "include/stack.h"
#include "include/val.h"
#include "../../include/backend/object.h"

static int ptr_set_val_get_dest(struct symbol *ident, enum ASF_REGS dest);
static int ptr_set_val_imm(enum ASF_REGS dest, enum OP_ID mode,
		struct asf_imm *src);
static int ptr_set_val_mem(enum ASF_REGS dest, enum OP_ID mode,
		struct asf_mem *src);
static int ptr_set_val_reg(enum ASF_REGS dest, enum OP_ID mode,
		enum ASF_REGS src);

int ptr_set_val_get_dest(struct symbol *ident, enum ASF_REGS dest)
{
	struct asf_mem mem = {};
	struct object_node *node;
	if (ident->type != SYM_IDENTIFIER)
		goto err_unsupport_sym;
	node = malloc(sizeof(*node));
	if ((node->s = asf_inst_mov_m2r(asf_stack_element2mem(
						ident->backend_status, &mem),
					dest)) == NULL)
		goto err_free_node;
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_unsupport_sym:
	printf("amc[backend.asf:%s]: ptr_set_val_get_dest: "
			"Unsupport symbol type\n",
			__FILE__);
	return 1;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return 1;
}

int ptr_set_val_imm(enum ASF_REGS dest, enum OP_ID mode, struct asf_imm *src)
{
	return 0;
}

int ptr_set_val_mem(enum ASF_REGS dest, enum OP_ID mode,
		struct asf_mem *src)
{
	return 0;
}

int ptr_set_val_reg(enum ASF_REGS dest, enum OP_ID mode, enum ASF_REGS src)
{
	return 0;
}

int asf_ptr_set_val(struct symbol *ident, yz_val *val, enum OP_ID mode)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	struct asf_val v = {};
	if (asf_val_get(val, &v))
		goto err_unsupport_type;
	if (ptr_set_val_get_dest(ident, dest))
		return 1;
	switch (v.type) {
	case ASF_VAL_IMM:
		return ptr_set_val_imm(dest, mode, &v.imm);
		break;
	case ASF_VAL_MEM:
		return ptr_set_val_mem(dest, mode, &v.mem);
		break;
	case ASF_VAL_REG:
		return ptr_set_val_reg(dest, mode, v.reg);
		break;
	default: break;
	}
err_unsupport_type:
	printf("amc[backend.asf:%s]: asf_ptr_set_val: "
			"Unsupport type: \"%s\"\n",
			__FILE__, yz_get_type_name(&val->type));
	return 1;
}
