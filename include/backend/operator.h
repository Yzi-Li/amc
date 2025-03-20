#ifndef AMC_BE_OPERATOR_H
#define AMC_BE_OPERATOR_H
#include "../expr.h"

typedef int (*backend_op_cmd_f)(struct expr *e);

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
