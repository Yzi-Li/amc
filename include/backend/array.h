/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ARRAY_H
#define AMC_BE_ARRAY_H
#include "symbol.h"
#include "../op.h"
#include "../val.h"

typedef int (*backend_array_def_f)(backend_symbol_status **raw_sym_stat,
		yz_val **vs, int len);
typedef int (*backend_array_set_elem_f)(struct symbol *sym, yz_val *offset,
		yz_val *val, enum OP_ID mode);

#endif
