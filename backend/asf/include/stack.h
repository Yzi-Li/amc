#ifndef AMC_BE_ASF_STACK_H
#define AMC_BE_ASF_STACK_H
#include "imm.h"
#include "register.h"

struct asf_stack_element {
	int addr;
	enum ASF_BYTES bytes;
	struct asf_stack_element *next, *prev;
};

extern struct asf_stack_element *asf_stack_top;

str *asf_inst_pop(enum ASF_REGS dest);
str *asf_inst_push(yz_val *val);
str *asf_inst_push_const(int src);
str *asf_inst_push_imm(struct asf_imm *src);
str *asf_inst_push_mem(struct asf_stack_element *src);
str *asf_inst_push_reg(enum ASF_REGS src);
void asf_stack_end_frame(struct asf_stack_element *start);
str *asf_stack_get_element(struct asf_stack_element *element, int pop);

#endif
