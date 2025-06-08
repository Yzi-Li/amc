#include "include/bytes.h"
#include "../../include/array.h"
#include "../../include/expr.h"

enum ASF_BYTES asf_bytes_get_size(enum ASF_BYTES bytes)
{
	if (bytes > ASF_BYTES_U_OFFSET)
		bytes -= ASF_BYTES_U_OFFSET;
	return bytes;
}

enum ASF_BYTES asf_yz_type_raw2bytes(enum YZ_TYPE type)
{
	switch (type) {
	case YZ_I8:
	case YZ_I16:
	case YZ_I32:
		return ASF_BYTES_32;
		break;
	case YZ_U8:
	case YZ_U16:
	case YZ_CHAR:
	case YZ_U32:
		return ASF_BYTES_U32;
		break;
	case YZ_I64: return ASF_BYTES_64; break;
	case YZ_PTR:
	case YZ_U64:
		return ASF_BYTES_U64;
		break;
	default: return ASF_BYTES_NULL; break;
	}
	return ASF_BYTES_NULL;
}

enum ASF_BYTES asf_yz_type2bytes(yz_val *type)
{
	if (type == NULL)
		return ASF_BYTES_NULL;
	switch (type->type) {
	case AMC_EXPR:
		return asf_yz_type_raw2bytes(
				*((struct expr*)type->v)->sum_type);
		break;
	case YZ_PTR:
		return ASF_BYTES_U64;
		break;
	case YZ_ARRAY:
		return asf_yz_type2bytes(&((yz_array*)type->v)->type);
		break;
	default:
		return asf_yz_type_raw2bytes(type->type);
		break;
	}
	return ASF_BYTES_NULL;
}
