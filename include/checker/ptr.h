/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_COMPTIME_PTR_H
#define AMC_COMPTIME_PTR_H
#include "../symbol.h"
#include "../val.h"

int check_ptr_can_null(yz_val *val, struct symbol *sym);
int check_ptr_can_ret(struct symbol *sym, struct symbol *fn);
int check_ptr_can_use(struct symbol *sym);
int check_ptr_get_addr_to_ident(struct expr *addr, struct symbol *ident);

#endif
