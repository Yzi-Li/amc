#ifndef AMC_PARSER_PTR_H
#define AMC_PARSER_PTR_H
#include "../../include/file.h"
#include "../../include/symbol.h"
#include "../../include/type.h"

int parse_type_ptr(struct file *f, yz_val *ptr, struct scope *scope);

#endif
