#ifndef AMC_BE_SCOPE_H
#define AMC_BE_SCOPE_H

typedef void backend_scope_status;

typedef backend_scope_status *(*backend_scope_begin_f)();
typedef int (*backend_scope_end_f)(backend_scope_status *raw_status);

#endif
