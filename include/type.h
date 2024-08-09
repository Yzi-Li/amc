#ifndef AMC_TYPE_H
#define AMC_TYPE_H
#include "../utils/cint.h"
#include "../utils/str/str.h"
#include "file.h"

// TODO: AMC_ERR_TYPE and AMC_USER_TYPE name improvement.
//       And pointer support.
enum YZ_TYPE {
	AMC_ERR_TYPE, AMC_USER_TYPE, AM_NULL, AMC_SUB_EXPR,
	AMC_SYM,
	YZ_VOID, YZ_CHAR,
	YZ_I8, YZ_I16, YZ_I32, YZ_I64,
	YZ_U8, YZ_U16, YZ_U32, YZ_U64,
	YZ_PTR // ptr start
};

static const unsigned int AM_TYPE_OFFSET = 6;

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

static struct yz_type_group const yz_type_table[] = {
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
enum YZ_TYPE yz_type_get(str *s);
int parse_type(str *token, enum YZ_TYPE *type);

#endif
