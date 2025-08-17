/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_STRUCT_H
#define AMC_BE_STRUCT_H
#include "symbol.h"
#include "../op.h"
#include "../struct.h"
#include "../val.h"

typedef int (*backend_struct_def_f)(backend_symbol_status *raw_sym_stat,
		yz_val **vs, int len);
typedef int (*backend_struct_set_elem_f)(struct symbol *ident, int index,
		yz_val *val, enum OP_ID mode);
typedef int (*backend_struct_set_elem_from_ptr_f)(struct symbol *ident,
		int index, yz_val *val, enum OP_ID mode);

#endif
