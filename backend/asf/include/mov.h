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

str *asf_inst_mov(enum ASF_MOV_TYPE mt, void *l, void *r);

#endif
