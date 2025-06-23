#ifndef AMC_PARSER_OP_H
#define AMC_PARSER_OP_H
#include "../../include/expr.h"
#include "../../include/symbol.h"

int op_apply_cmp(struct expr *e);
int op_apply_special(struct expr *e, struct scope *scope);
int op_assign(struct file *f, struct expr *e, struct scope *scope);
struct expr *op_extract_val_expr_create(enum YZ_TYPE *sum_type,
		yz_extract_val *val);

#endif
