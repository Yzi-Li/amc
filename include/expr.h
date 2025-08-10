/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_EXPR_H
#define AMC_EXPR_H
#include "op.h"
#include "val.h"
#include "../include/type.h"
#include <stdio.h>

#define EXPR_IS_SINGLE_TERM(EXPR) (\
		(EXPR)->op == NULL\
		&& (EXPR)->vall != NULL\
		&& (EXPR)->valr == NULL)

#define EXPR_IS_UNARY(EXPR) (\
		(EXPR)->op != NULL\
		&& (EXPR)->vall == NULL\
		&& (EXPR)->valr != NULL)

struct expr_operator {
	const char *sym;
	int priority;
	enum OP_ID id;
};

struct expr {
	struct expr_operator *op;
	yz_type *sum_type;
	yz_val *vall, *valr;
};

/**
 * @important: Won't create a new yz_val to contain the expression's val.
 *             But will create a new 'yz_val' to contain the expression.
 *             So you need free the result.
 */
yz_val *expr2yz_val(struct expr *expr);

void free_expr(struct expr *e);

#endif
