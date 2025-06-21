#ifndef AMC_BE_EXPR_H
#define AMC_BE_EXPR_H
#include "../op.h"
#include "../symbol.h"
#include "../type.h"

typedef int (*backend_var_immut_init_f)(struct symbol *sym, yz_val *val);
typedef int (*backend_var_set_f)(struct symbol *sym, enum OP_ID mode,
		yz_val *val);

#endif
