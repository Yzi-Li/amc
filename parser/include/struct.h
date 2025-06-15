#ifndef AMC_PARSER_STRUCT_H
#define AMC_PARSER_STRUCT_H
#include "../../include/struct.h"
#include "../../include/symbol.h"
#include "../../utils/str/str.h"

int constructor_struct(struct file *f, struct symbol *sym, struct scope *scope);
int parse_type_struct(str *token, yz_val *type, struct scope *scope);
int struct_get_elem(struct file *f, yz_val *val, struct scope *scope);
yz_struct *struct_type_find(str *s, struct scope *scope);
yz_struct *struct_type_get(str *s, yz_struct **structures, int count);

#endif
