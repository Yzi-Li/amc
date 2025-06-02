#ifndef AMC_COMPTIME_PTR_H
#define AMC_COMPTIME_PTR_H
#include "../symbol.h"
#include "../type.h"

int comptime_ptr_check_can_null(yz_val *val, struct symbol *sym);
int comptime_ptr_check_can_ret(struct symbol *sym, struct symbol *fn);
int comptime_ptr_check_can_use(struct symbol *sym);

#endif
