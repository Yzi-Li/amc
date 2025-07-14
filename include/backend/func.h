#ifndef AMC_BE_FUNC_H
#define AMC_BE_FUNC_H
#include "../type.h"

typedef int (*backend_func_call_f)(struct symbol *fn, yz_val **vs, int vlen);
typedef int (*backend_func_def_f)(struct symbol *fn);
typedef int (*backend_func_ret_f)(yz_val *v, int is_main);

typedef int (*backend_syscall_f)(int code);

#endif
