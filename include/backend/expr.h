#ifndef AMC_BE_EXPR_H
#define AMC_BE_EXPR_H
#include "../type.h"
#include "../../utils/str/str.h"

typedef int (*backend_var_immut_set_f)(str *name, yz_val *val);
typedef int (*backend_var_set_f)(str *name, yz_val *val);

/*
int backend_immut_var_set(str *name, yz_val *val);
int backend_var_set(str *name, yz_val *val);
*/

#endif
