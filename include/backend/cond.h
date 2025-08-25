/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_COND_H
#define AMC_BE_COND_H
#include "scope.h"
#include "../val.h"

typedef void backend_cond_match_handle;

typedef int (*backend_cond_elif_f)(backend_scope_status *raw_status);
typedef int (*backend_cond_else_f)(backend_scope_status *raw_status);
typedef int (*backend_cond_if_f)(backend_scope_status *raw_status);
typedef int (*backend_cond_if_begin_f)(backend_scope_status *raw_status);

typedef backend_cond_match_handle *(*backend_cond_match_begin_f)(yz_val *val);
typedef int (*backend_cond_match_case_f)(backend_cond_match_handle *handle,
		yz_val *val);
typedef int (*backend_cond_match_case_end_f)(
		backend_cond_match_handle *handle);
typedef int (*backend_cond_match_end_f)(backend_cond_match_handle *handle);
typedef void (*backend_cond_match_free_handle_f)(
		backend_cond_match_handle *handle);

#endif
