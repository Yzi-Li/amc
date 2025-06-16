#ifndef AMC_BE_ASF_VAL_H
#define AMC_BE_ASF_VAL_H
#include "imm.h"
#include "register.h"
#include "stack.h"
#include "../../../include/type.h"

enum ASF_VAL_TYPE {
	ASF_VAL_CONST,
	ASF_VAL_IMM,
	ASF_VAL_MEM,
	ASF_VAL_REG
};

struct asf_val {
	enum ASF_VAL_TYPE type;
	union {
		int const_id;
		struct asf_imm imm;
		struct asf_stack_element *mem;
		enum ASF_REGS reg;
	};
};

int asf_val_get(yz_val *src, struct asf_val *result);

#endif
