#ifndef AMC_BE_OPERATOR_H
#define AMC_BE_OPERATOR_H
#include "../type.h"

typedef int (*backend_op_cmd_f)(yz_val *l,
		yz_val *r);

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

/*
int backend_cmd_add(yz_val *l, yz_val *r, struct file *f);
int backend_cmd_sub(yz_val *l, yz_val *r, struct file *f);
int backend_cmd_div(yz_val *l, yz_val *r, struct file *f);
int backend_cmd_mul(yz_val *l, yz_val *r, struct file *f);

int backend_op_and(yz_val *l, yz_val *r, struct file *f);
int backend_op_eq(yz_val *l, yz_val *r, struct file *f);
int backend_op_ge(yz_val *l, yz_val *r, struct file *f);
int backend_op_gt(yz_val *l, yz_val *r, struct file *f);
int backend_op_le(yz_val *l, yz_val *r, struct file *f);
int backend_op_lt(yz_val *l, yz_val *r, struct file *f);
int backend_op_ne(yz_val *l, yz_val *r, struct file *f);
int backend_op_not(yz_val *l, yz_val *r, struct file *f);
int backend_op_or(yz_val *l, yz_val *r, struct file *f);
*/

#endif
