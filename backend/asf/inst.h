#ifndef AMC_BE_INST_H
#define AMC_BE_INST_H
#include "imm.h"

struct inst {
	const char *code;
};

enum ASF_MOV_TYPE {
	ASF_MOV_I2M,
	ASF_MOV_I2R,
	ASF_MOV_M2M,
	ASF_MOV_M2R,
	ASF_MOV_R2M,
	ASF_MOV_R2R
};

str *asf_inst_mov(enum ASF_MOV_TYPE mt, void *l, void *r);
str *asf_inst_pop(enum ASF_IMM_TYPE bytes, const char *dest);
str *asf_inst_push(enum ASF_IMM_TYPE bytes, const char *src);
str *asf_inst_pushi(struct asf_imm *imm);

#endif
