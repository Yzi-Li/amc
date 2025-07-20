#ifndef AMC_PARSER_ARRAY_H
#define AMC_PARSER_ARRAY_H
#include "../../include/op.h"
#include "../../include/symbol.h"

int array_get_elem(struct parser *parser, yz_val *val);
int array_set_elem(struct parser *parser, struct symbol *sym, yz_val *offset,
		enum OP_ID mode);
int constructor_array(struct parser *parser, struct symbol *sym);
int parse_type_array(struct parser *parser, yz_type *type);

#endif
