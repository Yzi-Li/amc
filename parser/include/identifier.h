#ifndef AMC_PARSER_IDENTIFIER_H
#define AMC_PARSER_IDENTIFIER_H
#include "../../include/expr.h"
#include "../../include/symbol.h"

yz_val *identifier_expr_val_handle(struct expr **e, yz_val *type);
int identifier_read(struct file *f, str *token, yz_val *val,
		struct scope *scope);

#endif
