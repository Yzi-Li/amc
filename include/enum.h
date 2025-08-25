/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_ENUM_H
#define AMC_ENUM_H
#include "type.h"
#include "../utils/cint.h"

union yz_enum_item_data {
	i64 s;
	u64 u;
};

typedef struct yz_enum_item {
	union yz_enum_item_data data;
	str name;
} yz_enum_item;

typedef struct yz_enum {
	int count;
	yz_enum_item **elems;
	str name;
	yz_type type;
} yz_enum;

void free_yz_enum(yz_enum *self);
void free_yz_enum_item(yz_enum_item *self);

#endif
