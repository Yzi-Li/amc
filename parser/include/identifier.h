#ifndef AMC_PARSER_IDENTIFIER_H
#define AMC_PARSER_IDENTIFIER_H
#include "../../include/expr.h"
#include "../../include/symbol.h"

int identifier_assign_val(struct file *f, struct symbol *sym, enum OP_ID mode,
		struct scope *scope);
yz_val *identifier_expr_val_handle(struct expr **e, yz_val *type);
int identifier_read(struct file *f, str *token, yz_val *val,
		struct scope *scope);

#endif
