#ifndef AMC_BE_ASF_BYTES_H
#define AMC_BE_ASF_BYTES_H
#include "../../../include/type.h"

enum ASF_BYTES {
	ASF_BYTES_NULL = 0,

	ASF_BYTES_8  = 1,
	ASF_BYTES_16 = 2,
	ASF_BYTES_32 = 4,
	ASF_BYTES_64 = 8,

	ASF_BYTES_U8  = 8 + 1,
	ASF_BYTES_U16 = 8 + 2,
	ASF_BYTES_U32 = 8 + 4,
	ASF_BYTES_U64 = 8 + 8,

	ASF_BYTES_PTR = 8 + 9
};

static const int ASF_BYTES_U_OFFSET = 8;

enum ASF_BYTES asf_yz_type_raw2bytes(enum YZ_TYPE type);
enum ASF_BYTES asf_yz_type2bytes(yz_val *type);

#endif
