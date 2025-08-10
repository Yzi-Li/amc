/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_NULL_H
#define AMC_BE_NULL_H
#include "../val.h"

typedef void backend_null_handle;

typedef int (*backend_null_handle_begin_f)(backend_null_handle **handle,
		yz_val *val);
typedef int (*backend_null_handle_end_f)(backend_null_handle *handle);

#endif
