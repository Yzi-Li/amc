#ifndef AMC_PARSER_IDENTIFIER_H
#define AMC_PARSER_IDENTIFIER_H
#include "../../include/expr.h"
#include "../../include/file.h"
#include "../../include/symbol.h"

int identifier_assign_get_val(struct parser *parser, yz_type *dest_type,
		yz_val **result);
int identifier_assign_val(struct parser *parser, struct symbol *sym,
		enum OP_ID mode);
int identifier_check_mut(struct file *f);
yz_val *identifier_expr_val_handle(struct expr **e, yz_type *type);
int identifier_read(struct parser *parser, yz_val *val);

#endif
