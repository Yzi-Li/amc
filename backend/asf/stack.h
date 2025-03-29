#ifndef AMC_BE_ASF_STACK_H
#define AMC_BE_ASF_STACK_H
#include "imm.h"

struct asf_stack_element {
	int addr;
	enum ASF_IMM_TYPE bytes;
	struct asf_stack_element *next, *prev;
};

extern struct asf_stack_element *asf_stack_top;

str *asf_stack_get_element(struct asf_stack_element *element, int pop);

#endif
