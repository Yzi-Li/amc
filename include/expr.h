#ifndef AMC_EXPR_H
#define AMC_EXPR_H
#include "op.h"
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
	enum YZ_TYPE *sum_type;
	yz_val *vall, *valr;
};

void free_expr(struct expr *e);

#endif
