#ifndef AMC_BE_ASF_IMM_H
#define AMC_BE_ASF_IMM_H
#include "bytes.h"
#include "../../../utils/cint.h"
#include "../../../utils/str/str.h"

struct asf_imm {
	union {
		i8  ib;
		i16 iw;
		i32 il;
		i64 iq;
	};
	enum ASF_BYTES type;
};

str *asf_imm_str_new(struct asf_imm *imm);

#endif
