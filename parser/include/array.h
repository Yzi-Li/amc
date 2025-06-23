#ifndef AMC_PARSER_ARRAY_H
#define AMC_PARSER_ARRAY_H
#include "../../include/op.h"
#include "../../include/symbol.h"

int array_get_elem(struct file *f, yz_val *val, struct scope *scope);
int array_set_elem(struct file *f, struct symbol *sym, yz_val *offset,
		enum OP_ID mode, struct scope *scope);
int constructor_array(struct file *f, struct symbol *sym, struct scope *scope);
int parse_type_array(struct file *f, yz_val *type, struct scope *scope);

#endif
