/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_CONST_H
#define AMC_BE_CONST_H
#include "../val.h"

typedef struct backend_const {
	yz_val val;
	void *data;
} backend_const;

/**
 * @return:
 *   const id.
 *   -1: error.
 */
typedef int (*backend_const_def_str_f)(backend_const *self, str *s);

#endif
