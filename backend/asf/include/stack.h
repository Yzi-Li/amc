/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_STACK_H
#define AMC_BE_ASF_STACK_H
#include "imm.h"
#include "mem.h"
#include "register.h"
#include "../../../include/backend/object.h"
#include "../../../include/val.h"

struct asf_stack_element {
	int addr, used;
	enum ASF_BYTES bytes;
	struct asf_stack_element *next, *prev;
};

extern struct asf_stack_element *asf_stack_top;

str *asf_inst_pop(enum ASF_REGS dest);
str *asf_inst_push(yz_val *val);
str *asf_inst_push_const(int src);
str *asf_inst_push_imm(struct asf_imm *src);
str *asf_inst_push_mem(struct asf_mem *src);
str *asf_inst_push_reg(enum ASF_REGS src);
int asf_stack_align(struct object_node *start_node);
struct asf_mem *asf_stack_element2mem(struct asf_stack_element *src,
		struct asf_mem *dest);
int asf_stack_end_frame(struct asf_stack_element *start_stack);
str *asf_stack_get_element(struct asf_mem *mem, int pop);

/**
 * Free all stack element from `start` while start->next != NULL.
 */
void free_asf_stack(struct asf_stack_element *start);

/**
 * Like `free_asf_stack` but reverse.
 * Free all stack element from `start` while start->prev != NULL.
 */
void free_asf_stack_reverse(struct asf_stack_element *start);

#endif
