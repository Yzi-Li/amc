/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_DECORATOR_H
#define AMC_BE_DECORATOR_H
#include "../../utils/str/str.h"

typedef int (*backend_dec_c_fn_f)(str *name, int argc);
typedef int (*backend_dec_syscall_f)(int code, int argc);

#endif
