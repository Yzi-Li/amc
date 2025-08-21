/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_CONST_H
#define AMC_CONST_H
#include "backend/const.h"
#include "val.h"

typedef struct yz_const {
	backend_const be_data;
	yz_val val;
} yz_const;

void free_yz_const(yz_const *self);

#endif
