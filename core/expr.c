/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/expr.h"

yz_val *expr2yz_val(struct expr *expr)
{
	yz_val *result = NULL;
	if (expr->op == NULL && expr->valr == NULL) {
		result = expr->vall;
		free(expr);
		return result;
	}
	result = malloc(sizeof(*result));
	result->v = expr;
	result->type.type = AMC_EXPR;
	result->type.v = result->v;
	return result;
}

void free_expr(struct expr *e)
{
	if (e == NULL)
		return;
	free_yz_val(e->vall);
	free_yz_val(e->valr);
	free_safe(e->op);
	free(e);
}
