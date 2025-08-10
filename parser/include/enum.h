/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PARSER_ENUM_H
#define AMC_PARSER_ENUM_H
#include "../../include/enum.h"
#include "../../include/scope.h"

yz_enum *yz_enum_find(str *s, struct scope *scope);
yz_enum_item *yz_enum_item_find(str *s, yz_enum *src);

#endif
