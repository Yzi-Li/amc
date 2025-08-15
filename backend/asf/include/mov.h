/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_MOV_H
#define AMC_BE_ASF_MOV_H
#include "../../../utils/str/str.h"

enum ASF_MOV_TYPE {
	ASF_MOV_C2M,
	ASF_MOV_C2R,
	ASF_MOV_I2M,
	ASF_MOV_I2R,
	ASF_MOV_M2M,
	ASF_MOV_M2R,
	ASF_MOV_R2M,
	ASF_MOV_R2R
};

enum ASF_MOV_MEM_TYPE {
	ASF_MOV_MEM_INREG_2_REG
};

str *asf_inst_mov(enum ASF_MOV_TYPE mt, void *l, void *r);
str *asf_inst_mov_mem(enum ASF_MOV_MEM_TYPE type, int offset, void *l, void *r);

#endif
