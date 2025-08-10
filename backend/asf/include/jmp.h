/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_JMP_H
#define AMC_BE_ASF_JMP_H
#include "../../../utils/str/str.h"

enum ASF_JMP_TYPE {
	ASF_JMP_ALWAYS,
	ASF_JMP_EQ,
	ASF_JMP_GE,
	ASF_JMP_GT,
	ASF_JMP_LE,
	ASF_JMP_LT,
	ASF_JMP_NE
};

str *asf_inst_jmp(enum ASF_JMP_TYPE inst, const char *label, int label_len);

#endif
