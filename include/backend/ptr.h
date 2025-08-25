/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_PTR_H
#define AMC_BE_PTR_H
#include "../op.h"
#include "../symbol.h"
#include "../val.h"

typedef int (*backend_ptr_set_val_f)(struct symbol *ident, yz_val *val,
		enum OP_ID mode);

#endif
