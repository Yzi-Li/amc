#ifndef AMC_COMPTIME_SYMBOL_H
#define AMC_COMPTIME_SYMBOL_H
#include "../symbol.h"

int comptime_check_sym_can_assign_val(struct symbol *sym, yz_val *val);
int comptime_check_sym_can_assign(struct symbol *sym);

#endif
