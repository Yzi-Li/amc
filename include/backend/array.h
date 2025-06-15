#ifndef AMC_BE_ARRAY_H
#define AMC_BE_ARRAY_H
#include "symbol.h"
#include "../type.h"

typedef int (*backend_array_def_f)(backend_symbol_status **raw_sym_stat,
		yz_val **vs, int len);
typedef int (*backend_array_get_elem_f)(backend_symbol_status *raw_sym_stat,
		yz_val *offset);

#endif
