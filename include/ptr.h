/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PTR_H
#define AMC_PTR_H
#include "val.h"

typedef struct yz_ptr {
	yz_val ref;
} yz_ptr;

typedef struct yz_ptr_type {
	yz_type ref;
	int level;
	unsigned int flag_can_null:1,
	             flag_checked_null:1,
	             flag_mut:1;
} yz_ptr_type;

yz_type *yz_type_max_ptr(yz_type *lraw, yz_type *rraw, yz_type *l, yz_type *r);

void free_yz_ptr(yz_ptr *self);
void free_yz_ptr_type(yz_ptr_type *self);

#endif
