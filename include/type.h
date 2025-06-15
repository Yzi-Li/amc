#ifndef AMC_TYPE_H
#define AMC_TYPE_H
#include "../utils/cint.h"
#include "../utils/str/str.h"
#include "../utils/utils.h"

enum YZ_TYPE {
	AMC_ERR_TYPE,
	AMC_SYM, AMC_EXPR,
	YZ_STRUCT, YZ_PTR, YZ_ARRAY, YZ_CONST, YZ_NULL,
	YZ_VOID, YZ_CHAR,
	YZ_I8, YZ_I16, YZ_I32, YZ_I64,
	YZ_U8, YZ_U16, YZ_U32, YZ_U64,
};

#define YZ_IS_DIGIT(X) (REGION_INT((X), YZ_I8, YZ_U64))
#define YZ_IS_UNSIGNED_DIGIT(X) (REGION_INT((X), YZ_U8, YZ_U64))
#define YZ_UNSIGNED_TO_SIGNED(X) ((X) - 4)

static const unsigned int YZ_TYPE_OFFSET = 8;

/**
 * @field type: For builtin type.
 * @field len: The number of bytes used for type.
 */
struct yz_type_group {
	const char *name;
	enum YZ_TYPE type;
	int len;
};

typedef struct yz_val {
	union {
		void *v;
		char *s;
		i8 b;
		i16 w;
		i32 i;
		i64 l;
	};

	enum YZ_TYPE type;
} yz_val;

static const struct yz_type_group yz_type_table[] = {
	{"void",  YZ_VOID,  0},
	{"char",  YZ_CHAR,  1},
	{"i8",    YZ_I8,    1},
	{"i16",   YZ_I16,   2},
	{"i32",   YZ_I32,   4},
	{"i64",   YZ_I64,   8},
	{"u8",    YZ_U8,    1},
	{"u16",   YZ_U16,   2},
	{"u32",   YZ_U32,   4},
	{"u64",   YZ_U64,   8},
};

enum YZ_TYPE yz_get_int_size(long long l);
enum YZ_TYPE *yz_get_raw_type(yz_val *val);
const char *yz_get_type_name(yz_val *val);
enum YZ_TYPE yz_type_get(str *s);
yz_val *yz_type_max(yz_val *l, yz_val *r);

#endif
