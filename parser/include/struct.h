#ifndef AMC_PARSER_STRUCT_H
#define AMC_PARSER_STRUCT_H
#include "../../include/op.h"
#include "../../include/struct.h"
#include "../../include/symbol.h"
#include "../../include/val.h"
#include "../../utils/str/str.h"

int constructor_struct(struct parser *parser, struct symbol *sym);
int parse_type_struct(str *token, yz_type *result, struct scope *scope);
int struct_get_elem(struct parser *parser, yz_val *val);
int struct_set_elem(struct parser *parser, struct symbol *sym, int index,
		enum OP_ID mode);
yz_struct *struct_type_find(str *s, struct scope *scope);

#endif
