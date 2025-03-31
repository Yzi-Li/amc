#ifndef ASF_BE_CALL_H
#define ASF_BE_CALL_H
#include "register.h"

static enum ASF_REGS asf_call_arg_regs[] = {
	ASF_REG_RDI,
	ASF_REG_RSI,
	ASF_REG_RDX,
	ASF_REG_RCX,
	ASF_REG_R8,
	ASF_REG_R9
};
static const int asf_call_arg_regs_len = LENGTH(asf_call_arg_regs);

#endif
