#ifndef AMC_PARSER_FUNC_H
#define AMC_PARSER_FUNC_H
#include "../../include/file.h"
#include "../../include/symbol.h"

int func_call_read(struct file *f, struct symbol **fn, struct scope *scope);

#endif
