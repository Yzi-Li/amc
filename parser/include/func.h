/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PARSER_FUNC_H
#define AMC_PARSER_FUNC_H
#include "../../include/parser.h"
#include "../../include/val.h"

int func_call_read(struct parser *parser, yz_val *val, struct symbol *fn);

#endif
