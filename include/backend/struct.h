#ifndef AMC_BE_STRUCT_H
#define AMC_BE_STRUCT_H
#include "symbol.h"
#include "../op.h"
#include "../struct.h"
#include "../type.h"

typedef int (*backend_struct_def_f)(backend_symbol_status *raw_sym_stat,
		yz_val **vs, int len);
typedef int (*backend_struct_get_elem_f)(backend_symbol_status *raw_sym_stat,
		yz_struct *src, int index);
typedef int (*backend_struct_set_elem_f)(struct symbol *sym, int index,
		yz_val *val, enum OP_ID mode);

#endif
