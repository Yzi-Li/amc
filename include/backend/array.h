#ifndef AMC_BE_ARRAY_H
#define AMC_BE_ARRAY_H
#include "../array.h"
#include "../type.h"

typedef int (*backend_array_def_f)(char *name, yz_val **vs, int len);

/**
 * @important: Name mustn't be freed!
 */
typedef int (*backend_array_get_elem_f)(char *name, yz_val *offset);

#endif
