/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PARSER_SYMBOL_H
#define AMC_PARSER_SYMBOL_H
#include "../../include/file.h"
#include "../../utils/str/str.h"

int symbol_read(str *result, struct file *f);

#endif
