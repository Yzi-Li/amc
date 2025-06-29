#ifndef AMC_BE_COND_H
#define AMC_BE_COND_H
#include "scope.h"

typedef int (*backend_cond_elif_f)(backend_scope_status *raw_status);
typedef int (*backend_cond_else_f)(backend_scope_status *raw_status);
typedef int (*backend_cond_if_f)(backend_scope_status *raw_status);
typedef int (*backend_cond_if_begin_f)(backend_scope_status *raw_status);

#endif
