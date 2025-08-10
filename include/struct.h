/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_STRUCT_H
#define AMC_STRUCT_H
#include "../utils/str/str.h"

struct yz_struct_flag {
	unsigned int mut:1, rec:1;
};

struct symbol;
typedef struct yz_struct {
	struct symbol **elems;
	int elem_count;
	struct yz_struct_flag flags;
	str name;
} yz_struct;

void free_yz_struct(yz_struct *src);

#endif
