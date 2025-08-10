/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_CALL_H
#define AMC_BE_ASF_CALL_H
#include "register.h"
#include "../../../include/val.h"

static enum ASF_REGS asf_call_arg_regs[] = {
	ASF_REG_RDI,
	ASF_REG_RSI,
	ASF_REG_RDX,
	ASF_REG_RCX,
	ASF_REG_R8,
	ASF_REG_R9
};
static const int asf_call_arg_regs_len = LENGTH(asf_call_arg_regs);

int asf_call_push_args(int vlen, yz_val **vs);
str *asf_inst_syscall(int code, int vlen, yz_val **vs);

#endif
