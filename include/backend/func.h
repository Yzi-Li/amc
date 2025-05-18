#ifndef AMC_BE_FUNC_H
#define AMC_BE_FUNC_H
#include "../type.h"

typedef int (*backend_func_call_f)(const char *name, yz_val *type,
		yz_val **vs, int vlen);
typedef int (*backend_func_def_f)(const char *name, int len, yz_val *type);
typedef int (*backend_func_ret_f)(yz_val *v, int is_main);

#endif
