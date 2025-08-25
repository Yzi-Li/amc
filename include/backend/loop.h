/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_LOOP_H
#define AMC_BE_LOOP_H

typedef void backend_while_handle;

typedef backend_while_handle *(*backend_while_begin_f)(void);
typedef int (*backend_while_end_f)(backend_while_handle *handle);
typedef int (*backend_while_cond_f)(backend_while_handle *handle);
typedef void (*backend_while_free_handle_f)(backend_while_handle *handle);

#endif
