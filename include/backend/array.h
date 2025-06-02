#ifndef AMC_BE_ARRAY_H
#define AMC_BE_ARRAY_H
#include "scope.h"
#include "../type.h"

/**
 * @important: 'name' will be freed!
 */
typedef int (*backend_array_def_f)(char *name, yz_val **vs, int len,
		backend_scope_status *raw_status);

/**
 * @important: 'name' will be freed!
 */
typedef int (*backend_array_get_elem_f)(char *name, yz_val *offset);

#endif
