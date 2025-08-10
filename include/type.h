/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_TYPE_H
#define AMC_TYPE_H
#include "../utils/str/str.h"
#include "../utils/utils.h"
#include <limits.h>

enum YZ_TYPE {
	AMC_ERR_TYPE,
	AMC_SYM, AMC_EXPR, AMC_EXTRACT_VAL,

	YZ_ARRAY,
	YZ_CONST,
	YZ_ENUM,
	YZ_NULL,
	YZ_PTR,
	YZ_STRUCT,
	YZ_VOID,

	YZ_CHAR,
	YZ_I8, YZ_I16, YZ_I32, YZ_I64,
	YZ_U8, YZ_U16, YZ_U32, YZ_U64,
};

#define YZ_IS_DIGIT(X) (REGION_INT((X), YZ_I8, YZ_U64))
#define YZ_IS_UNSIGNED_DIGIT(X) (REGION_INT((X), YZ_U8, YZ_U64))
#define YZ_UNSIGNED_TO_SIGNED(X) ((X) - 4)

static const unsigned int YZ_TYPE_OFFSET = 9;

/**
 * @field type: For builtin type.
 * @field len: The number of bytes used for type.
 */
struct yz_type_group {
	const char *name;
	enum YZ_TYPE type;
	int len;
};

typedef struct yz_type {
	void *v;
	enum YZ_TYPE type;
} yz_type;

typedef struct yz_user_type {
	struct yz_user_type *nodes[UCHAR_MAX + 1];
	enum YZ_TYPE type;
	union {
		struct yz_enum *enum_;
		struct yz_struct *struct_;
	};
} yz_user_type;

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
yz_type *yz_get_raw_type(yz_type *type);
const char *yz_get_raw_type_name(enum YZ_TYPE type);
const char *yz_get_type_name(yz_type *type);
enum YZ_TYPE yz_type_get(str *s);
yz_type *yz_type_max(yz_type *l, yz_type *r);

void free_yz_type(yz_type *self);
void free_yz_type_noself(yz_type *self);
void free_yz_user_type(void *self);

#endif
