#ifndef AMC_BE_EXPR_H
#define AMC_BE_EXPR_H
#include "../type.h"

typedef int (*backend_var_immut_init_f)(char *name, yz_val *val);
typedef int (*backend_var_set_f)(char *name, yz_val *val);

#endif
