#ifndef AMC_PARSER_TYPE_H
#define AMC_PARSER_TYPE_H
#include "../../include/type.h"
#include "../../include/symbol.h"

int parse_type(struct parser *parser, yz_type *result);
int parse_type_name_pair(struct parser *parser, struct symbol *result);

#endif
