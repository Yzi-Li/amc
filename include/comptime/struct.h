/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_COMPTIME_STRUCT_H
#define AMC_COMPTIME_STRUCT_H
#include "../symbol.h"

int comptime_check_struct_elem_can_assign(struct symbol *sym,
		struct symbol *elem);

#endif
