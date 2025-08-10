/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/bytes.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/val.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct asf_stack_element *asf_stack_top = NULL;

static int stack_element_append(enum ASF_BYTES bytes);
static void stack_element_remove();

int stack_element_append(enum ASF_BYTES bytes)
{
	struct asf_stack_element *element = malloc(sizeof(*asf_stack_top));
	if (element == NULL)
		return 1;
	if (asf_stack_top == NULL) {
		asf_stack_top = element;
		asf_stack_top->next = NULL;
		asf_stack_top->prev = NULL;
		asf_stack_top->addr = asf_bytes_get_size(bytes);
		asf_stack_top->bytes = bytes;
		return 0;
	}
	element->next = NULL;
	element->prev = asf_stack_top;
	element->addr = element->prev->addr + asf_bytes_get_size(bytes);
	element->bytes = bytes;
	asf_stack_top->next = element;
	asf_stack_top = element;
	return 0;
}

void stack_element_remove()
{
	struct asf_stack_element *element = asf_stack_top;
	if (asf_stack_top->prev == NULL) {
		free_cl(asf_stack_top);
		return;
	}
	asf_stack_top->prev->next = NULL;
	asf_stack_top = asf_stack_top->prev;
	free(element);
}

str *asf_inst_pop(enum ASF_REGS dest)
{
	str *s = NULL;
	s = asf_inst_mov(ASF_MOV_M2R, asf_stack_top, &dest);
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
		return asf_inst_push_mem(v.mem);
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
	if (stack_element_append(ASF_BYTES_U64))
		return NULL;
	return asf_inst_mov(ASF_MOV_C2M, &src, asf_stack_top);
}

str *asf_inst_push_imm(struct asf_imm *src)
{
	if (stack_element_append(src->type))
		return NULL;
	return asf_inst_mov(ASF_MOV_I2M, src, asf_stack_top);
}

str *asf_inst_push_mem(struct asf_stack_element *src)
{
	if (stack_element_append(src->bytes))
		return NULL;
	return asf_inst_mov(ASF_MOV_M2M, src, asf_stack_top);
}

str *asf_inst_push_reg(enum ASF_REGS src)
{
	if (stack_element_append(asf_regs[src].bytes))
		return NULL;
	*asf_regs[src].purpose = ASF_REG_PURPOSE_NULL;
	return asf_inst_mov(ASF_MOV_R2M, &src, asf_stack_top);
}

void asf_stack_end_frame(struct asf_stack_element *start)
{
	struct asf_stack_element *cur = NULL, *next = NULL;
	if (start == NULL) {
		cur = asf_stack_top;
	} else {
		cur = start->next;
		if (start->prev != NULL)
			start->prev->next = NULL;
	}
	if ((asf_stack_top = start) == NULL)
		return;
	asf_stack_top->next = NULL;
	while (cur != NULL) {
		next = cur->next;
		free(cur);
		cur = next;
	}
	return;
}

str *asf_stack_get_element(struct asf_stack_element *element, int pop)
{
	str *s = str_new();
	const char *temp = "-%d(%%rbp)";
	str_expand(s, strlen(temp) - 2 + ullen(element->addr));
	snprintf(s->s, s->len, temp, element->addr);
	if (pop)
		stack_element_remove();
	return s;
}
