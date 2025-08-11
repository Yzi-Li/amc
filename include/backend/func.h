/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_FUNC_H
#define AMC_BE_FUNC_H
#include "../symbol.h"
#include "../val.h"

typedef int (*backend_func_call_f)(struct symbol *fn, yz_val **vs, int vlen);
typedef int (*backend_func_def_f)(struct symbol *fn, int pub, int main);
typedef int (*backend_func_ret_f)(yz_val *v, int is_main);

typedef int (*backend_syscall_f)(int code, int argc);

#endif
