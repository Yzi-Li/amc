#ifndef AMC_BE_LOOP_H
#define AMC_BE_LOOP_H
#include "scope.h"

typedef int (*backend_while_begin_f)(backend_scope_status *raw_status);
typedef int (*backend_while_end_f)(backend_scope_status *raw_status);

#endif
