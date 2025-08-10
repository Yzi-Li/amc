/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_MODULE_H
#define AMC_MODULE_H
#include "symbol.h"
#include "../utils/str/str.h"

struct yz_module;
typedef struct yz_module {
	str name, path;

	struct scope *scope;
} yz_module;


str *module_path2real(str *path);

int check_module_name(str *name);

void free_yz_module(yz_module *src);

#endif
