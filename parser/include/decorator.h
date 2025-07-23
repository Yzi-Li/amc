#ifndef AMC_PARSER_DECORATOR_H
#define AMC_PARSER_DECORATOR_H
#include "../../include/decorator.h"
#include "../../include/file.h"

struct decorator *get_decorator(str *token);
int parse_decorator(struct decorators *self, struct file *f);

void free_decorators_noself(struct decorators *self);

#endif
