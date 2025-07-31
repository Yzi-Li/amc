#ifndef AMC_ENUM_H
#define AMC_ENUM_H
#include "type.h"
#include "../utils/cint.h"

typedef struct yz_enum_item {
	str name;
	union {
		i64 s;
		u64 u;
	};
} yz_enum_item;

typedef struct yz_enum {
	int count;
	yz_enum_item **elems;
	str name;
	yz_type type;
} yz_enum;

void free_yz_enum(yz_enum *self);

#endif
