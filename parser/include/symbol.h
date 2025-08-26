/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PARSER_SYMBOL_H
#define AMC_PARSER_SYMBOL_H
#include "../../include/file.h"
#include "../../include/parser.h"
#include "../../utils/str/str.h"

int symbol_read(struct parser *parser, yz_val *val);
int symbol_read_name(str *result, struct file *f);

#endif
