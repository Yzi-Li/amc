#ifndef AMC_PARSER_ARRAY_H
#define AMC_PARSER_ARRAY_H
#include "../../include/symbol.h"

int array_get_elem(struct file *f, yz_val *val, struct scope *scope);
int array_structure(struct file *f, struct symbol *sym, struct scope *scope);
int array_structure_elem(const char *se, struct file *f, void *data);
int parse_type_array(struct file *f, yz_val *type);

#endif
