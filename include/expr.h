#ifndef AMC_EXPR_H
#define AMC_EXPR_H
#include "../include/type.h"

#define EXPR_IS_SINGLE_TERM(EXPR) (\
		(EXPR)->op == NULL\
		&& (EXPR)->vall != NULL\
		&& (EXPR)->valr == NULL)

#define EXPR_IS_UNARY(EXPR) (\
		(EXPR)->op != NULL\
		&& (EXPR)->vall == NULL\
		&& (EXPR)->valr != NULL)

enum OP_ID {
	OP_ADD,
	OP_AND,
	OP_DIV,
	OP_EQ,
	OP_GE,
	OP_GT,
	OP_LE,
	OP_LT,
	OP_MUL,
	OP_NE,
	OP_NOT,
	OP_OR,
	OP_SUB,

	OP_ASSIGN,
	OP_ASSIGN_ADD,
	OP_ASSIGN_DIV,
	OP_ASSIGN_MUL,
	OP_ASSIGN_SUB,

	OP_EXTRACT_VAL,
	OP_GET_ADDR
};

#define OP_LEN 20
#define OP_SPECIAL_START 13

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
#endif
