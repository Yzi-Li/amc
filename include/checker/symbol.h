/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_COMPTIME_SYMBOL_H
#define AMC_COMPTIME_SYMBOL_H
#include "../symbol.h"
#include "../val.h"

int check_sym_can_assign(struct symbol *sym);
int check_sym_can_assign_val(struct symbol *sym, yz_val *val);

#endif
