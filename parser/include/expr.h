#ifndef AMC_PARSER_EXPR_H
#define AMC_PARSER_EXPR_H
#include "../../include/expr.h"
#include "../../include/symbol.h"

int expr_apply(struct parser *parser, struct expr *e);
struct expr *parse_expr(struct parser *parser, int top);

#endif
