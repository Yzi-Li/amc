#ifndef AMC_PARSER_TYPE_H
#define AMC_PARSER_TYPE_H
#include "../../include/file.h"
#include "../../include/type.h"
#include "../../include/symbol.h"

int parse_type(struct file *f, yz_val *type, struct scope *scope);
int parse_type_name_pair(struct file *f, struct symbol *result,
		struct scope *scope);

#endif
