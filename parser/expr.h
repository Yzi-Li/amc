#ifndef AMC_PARSER_EXPR_H
#define AMC_PARSER_EXPR_H
#include "../include/expr.h"
#include "../include/file.h"
#include "../include/symbol.h"
#include "../include/type.h"

int expr_apply(struct expr *e);
void expr_free(struct expr *e);
struct expr *parse_expr(struct file *f, int top, struct scope *scope);

#endif
