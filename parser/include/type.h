/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PARSER_TYPE_H
#define AMC_PARSER_TYPE_H
#include "../../include/type.h"
#include "../../include/symbol.h"

int parse_type(struct parser *parser, yz_type *result);
int parse_type_name_pair(struct parser *parser, str *name, yz_type *type);
yz_user_type *yz_user_type_find(str *s, struct scope *scope);

#endif
