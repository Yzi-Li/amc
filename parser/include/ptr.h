/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PARSER_PTR_H
#define AMC_PARSER_PTR_H
#include "../../include/expr.h"
#include "../../include/op.h"
#include "../../include/symbol.h"
#include "../../include/type.h"

int parse_type_ptr(struct parser *parser, yz_type *result);

int ptr_set_val(struct parser *parser, struct symbol *ident, enum OP_ID mode);
int ptr_set_val_handle_expr(struct expr *expr);

#endif
