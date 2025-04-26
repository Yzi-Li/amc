#ifndef AMC_BE_FUNC_H
#define AMC_BE_FUNC_H
#include "../type.h"

typedef int (*backend_func_call_f)(const char *name, enum YZ_TYPE type,
		yz_val **vs, int vlen);
typedef int (*backend_func_def_f)(const char *name, int len, enum YZ_TYPE type);
typedef int (*backend_func_ret_f)(yz_val *v, int is_main);

/*
int backend_func_call(const char *name);
int backend_func_def(const char *name, int len, enum YZ_TYPE type);
int backend_func_ret(struct file *f, yz_val *v);
*/

#endif
