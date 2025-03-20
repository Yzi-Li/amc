#ifndef AMC_EXPR_H
#define AMC_EXPR_H
#include "../include/type.h"

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
	OP_NONE,
	OP_NOT,
	OP_OR,
	OP_SUB,

	OP_ASSIGNMENT
};

#define OP_LEN 15
static const int OP_SPECIAL_START = 14;

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
