/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_MEM_H
#define AMC_BE_ASF_MEM_H
#include "register.h"

struct asf_mem {
	enum ASF_REGS addr;
	enum ASF_BYTES bytes;
	int offset;
};

str *asf_mem_get_str(struct asf_mem *mem);

#endif
