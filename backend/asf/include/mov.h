/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_MOV_H
#define AMC_BE_ASF_MOV_H
#include "imm.h"
#include "mem.h"
#include "register.h"
#include "../../../utils/str/str.h"

union asf_mov_operand {
	int i;
	enum ASF_REGS reg;
	struct asf_imm *imm;
	void *v;
};

str *asf_inst_mov_c2m(int src, struct asf_mem *dest);
str *asf_inst_mov_c2r(int src, enum ASF_REGS dest);
str *asf_inst_mov_i2m(struct asf_imm *src, struct asf_mem *dest);
str *asf_inst_mov_i2r(struct asf_imm *src, enum ASF_REGS dest);
str *asf_inst_mov_m2m(struct asf_mem *src, struct asf_mem *dest);
str *asf_inst_mov_m2r(struct asf_mem *src, enum ASF_REGS dest);
str *asf_inst_mov_r2m(enum ASF_REGS src, struct asf_mem *dest);
str *asf_inst_mov_r2r(enum ASF_REGS src, enum ASF_REGS dest);

#endif
