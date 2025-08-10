/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_ARRAY_H
#define AMC_ARRAY_H
#include "type.h"

typedef struct yz_array {
	yz_type type;
	// when len == 0: mut array
	int len;
} yz_array;

yz_type *yz_type_max_arr(yz_type *l, yz_type *r);

#endif
