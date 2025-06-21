#ifndef AMC_COMPTIME_MUT_H
#define AMC_COMPTIME_MUT_H
#include "../symbol.h"

int comptime_check_struct_elem_can_assign(struct symbol *sym,
		struct symbol *elem);
int comptime_check_sym_can_assign(struct symbol *sym);

#endif
