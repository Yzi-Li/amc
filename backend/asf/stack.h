#ifndef AMC_BE_ASF_STACK_H
#define AMC_BE_ASF_STACK_H
#include "imm.h"

enum ASF_STACK_MODE {
	ASF_STACK_MODE_NATIVE,
	ASF_STACK_MODE_LOCAL
};

struct asf_stack_element {
	int addr;
	enum ASF_IMM_TYPE bytes;
	struct asf_stack_element *next, *prev;
};

extern enum ASF_STACK_MODE asf_stack_mode;
extern struct asf_stack_element *asf_stack_top;

#endif
