#ifndef AMC_BE_EXPR_H
#define AMC_BE_EXPR_H
#include "scope.h"
#include "../op.h"
#include "../type.h"

/**
 * @important: 'name' will be freed.
 */
typedef int (*backend_var_immut_init_f)(char *name, yz_val *val,
		backend_scope_status *raw_status);

/**
 * @important: 'name' will be freed.
 */
typedef int (*backend_var_set_f)(char *name, yz_val *val, enum OP_ID mode,
		backend_scope_status *raw_status);

#endif
