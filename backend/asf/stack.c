/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/bytes.h"
#include "include/mem.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/val.h"
#include "../../include/backend/object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int asf_stack_addr = 0;
struct asf_stack_element *asf_stack_top = NULL;

static int stack_element_append(enum ASF_BYTES bytes);
static void stack_element_remove();

int stack_element_append(enum ASF_BYTES bytes)
{
	if (asf_stack_top == NULL) {
		asf_stack_top = calloc(1, sizeof(*asf_stack_top));
		asf_stack_top->addr = asf_bytes_get_size(bytes);
		asf_stack_top->bytes = bytes;
		asf_stack_top->used = 1;
		asf_stack_addr = asf_stack_top->addr;
		return 0;
	}
	if (!asf_stack_top->used) {
		asf_stack_top->addr = asf_bytes_get_size(bytes);
		asf_stack_top->bytes = bytes;
		asf_stack_top->used = 1;
		if (asf_stack_addr < asf_stack_top->addr)
			asf_stack_addr = asf_stack_top->addr;
		return 0;
	}
	if (asf_stack_top->next == NULL) {
		asf_stack_top->next = calloc(1, sizeof(*asf_stack_top));
		asf_stack_top->next->prev = asf_stack_top;
	}
	asf_stack_top = asf_stack_top->next;
	asf_stack_top->addr = asf_stack_top->prev->addr
		+ asf_bytes_get_size(bytes);
	asf_stack_top->bytes = bytes;
	asf_stack_top->used = 1;
	if (asf_stack_addr < asf_stack_top->addr)
		asf_stack_addr = asf_stack_top->addr;
	return 0;
}

void stack_element_remove()
{
	if (asf_stack_top == NULL)
		return;
	asf_stack_top->used = 0;
	if (asf_stack_top->prev == NULL)
		return;
	asf_stack_top = asf_stack_top->prev;
}

str *asf_inst_pop(enum ASF_REGS dest)
{
	struct asf_mem mem = {};
	str *s = NULL;
	s = asf_inst_mov_m2r(asf_stack_element2mem(asf_stack_top, &mem), dest);
	stack_element_remove();
	return s;
}

str *asf_inst_push(yz_val *val)
{
	struct asf_val v = {};
	if (asf_val_get(val, &v))
		goto err_unsupport_type;
	if (v.type == ASF_VAL_CONST) {
		return asf_inst_push_const(v.const_id);
	} else if (v.type == ASF_VAL_IMM) {
		return asf_inst_push_imm(&v.imm);
	} else if (v.type == ASF_VAL_MEM) {
		return asf_inst_push_mem(&v.mem);
	} else if (v.type == ASF_VAL_REG) {
		return asf_inst_push_reg(v.reg);
	}
err_unsupport_type:
	printf("amc[backend.asf]: asf_inst_push: Unsupport type: '%s'!\n",
			yz_get_type_name(&val->type));
	return NULL;
}

str *asf_inst_push_const(int src)
{
	struct asf_mem mem = {};
	if (stack_element_append(ASF_BYTES_U64))
		return NULL;
	asf_stack_element2mem(asf_stack_top, &mem);
	return asf_inst_mov_c2m(src, &mem);
}

str *asf_inst_push_imm(struct asf_imm *src)
{
	struct asf_mem mem = {};
	if (stack_element_append(src->type))
		return NULL;
	asf_stack_element2mem(asf_stack_top, &mem);
	return asf_inst_mov_i2m(src, &mem);
}

str *asf_inst_push_mem(struct asf_mem *src)
{
	struct asf_mem right_operand = {};
	if (stack_element_append(src->bytes))
		return NULL;
	asf_stack_element2mem(asf_stack_top, &right_operand);
	return asf_inst_mov_m2m(src, &right_operand);
}

str *asf_inst_push_reg(enum ASF_REGS src)
{
	struct asf_mem mem = {};
	if (stack_element_append(asf_regs[src].bytes))
		return NULL;
	*asf_regs[src].purpose = ASF_REG_PURPOSE_NULL;
	asf_stack_element2mem(asf_stack_top, &mem);
	return asf_inst_mov_r2m(src, &mem);
}

int asf_stack_align(struct object_node *start_node)
{
	int align;
	const char *temp = "subq $%lld, %%rsp\n";
	struct object_node *node;
	if (asf_stack_addr == 0)
		return 0;
	if (start_node == NULL)
		return 1;
	align = (asf_stack_addr + 15) & -16;
	asf_stack_addr = 0;
	node = malloc(sizeof(*node));
	node->s = str_new();
	str_expand(node->s, strlen(temp) + ullen(align) - 4);
	snprintf(node->s->s, node->s->len, temp, align);
	if (object_insert(node, start_node, start_node->next))
		goto err_free_node_and_str;
	return 0;
err_free_node_and_str:
	str_free(node->s);
	free(node);
	return 1;
}

struct asf_mem *asf_stack_element2mem(struct asf_stack_element *src,
		struct asf_mem *dest)
{
	dest->addr = ASF_REG_RBP;
	dest->bytes = src->bytes;
	dest->offset = -(src->addr);
	return dest;
}

int asf_stack_end_frame(struct asf_stack_element *start_stack)
{
	struct asf_stack_element *cur = NULL, *next = NULL;
	if (start_stack == NULL) {
		stack_element_remove();
		return 0;
	}
	asf_stack_top = start_stack;
	if ((cur = start_stack->next) == NULL) {
		asf_stack_addr = 0;
		return 0;
	}
	while (cur != NULL) {
		next = cur->next;
		cur->used = 0;
		cur = next;
	}
	return 0;
}

str *asf_stack_get_element(struct asf_mem *mem, int pop)
{
	str *s = asf_mem_get_str(mem);
	if (pop)
		stack_element_remove();
	return s;
}

void free_asf_stack(struct asf_stack_element *start)
{
	struct asf_stack_element *cur = start, *nex;
	if (start == NULL)
		return;
	while (cur != NULL) {
		nex = cur->next;
		free(cur);
		cur = nex;
	}
}

void free_asf_stack_reverse(struct asf_stack_element *start)
{
	struct asf_stack_element *cur = start, *nex;
	if (start == NULL)
		return;
	while (cur != NULL) {
		nex = cur->prev;
		free(cur);
		cur = nex;
	}
}
