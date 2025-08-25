/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_SCOPE_H
#define AMC_BE_SCOPE_H

typedef void backend_scope_status;

typedef backend_scope_status *(*backend_scope_begin_f)(void);
typedef int (*backend_scope_end_f)(backend_scope_status *raw_status);
typedef void (*backend_scope_free_f)(backend_scope_status *raw_status);

#endif
