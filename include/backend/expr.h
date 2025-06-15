#ifndef AMC_BE_EXPR_H
#define AMC_BE_EXPR_H
#include "symbol.h"
#include "../op.h"
#include "../type.h"

typedef int (*backend_var_immut_init_f)(backend_symbol_status **raw_sym_stat,
		yz_val *val);

typedef int (*backend_var_set_f)(backend_symbol_status **raw_sym_stat,
		enum OP_ID mode, yz_val *val);

#endif
