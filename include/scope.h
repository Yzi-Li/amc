#ifndef AMC_SCOPE_H
#define AMC_SCOPE_H
#include "symbol.h"

int scope_check_is_correct(struct scope *scope);
int scope_end(struct scope *scope);

#endif
