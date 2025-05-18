#ifndef AMC_BE_ASF_IMM_H
#define AMC_BE_ASF_IMM_H
#include "../../../include/type.h"
#include "../../../utils/cint.h"
#include "../../../utils/str/str.h"

static const int ASF_IMM_UNSIGNED_OFFSET = 8;

enum ASF_IMM_TYPE {
	ASF_IMM_NULL = 0,

	ASF_IMM8  = 1,
	ASF_IMM16 = 2,
	ASF_IMM32 = 4,
	ASF_IMM64 = 8,

	ASF_IMMU8  = 8 + 1,
	ASF_IMMU16 = 8 + 2,
	ASF_IMMU32 = 8 + 4,
	ASF_IMMU64 = 8 + 8,

	ASF_IMM_PTR = 8 + 9
};

struct asf_imm {
	union {
		i8  ib;
		i16 iw;
		i32 il;
		i64 iq;
	};
	enum ASF_IMM_TYPE type;
};

enum ASF_IMM_TYPE asf_yz_type_raw2imm(enum YZ_TYPE type);
enum ASF_IMM_TYPE asf_yz_type2imm(yz_val *type);
str *asf_imm_str_new(struct asf_imm *imm);

#endif
