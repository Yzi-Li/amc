#ifndef AMC_BE_ASF_H
#define AMC_BE_ASF_H
#include "cond.h"
#include "scope.h"
#include "../../../include/backend.h"

enum OBJ_SECTION_TYPE {
	ASF_OBJ_TEXT,
	ASF_OBJ_DATA,
	ASF_OBJ_RODATA
};

int asf_end();
int asf_file_new(struct file *f);
int asf_init(int argc, char *argv[]);
int asf_stop(enum BE_STOP_SIGNAL bess);

int asf_cond_elif(backend_scope_status *raw_status);
int asf_cond_else(backend_scope_status *raw_status);
int asf_cond_if(backend_scope_status *raw_status);

int asf_func_call(const char *name, enum YZ_TYPE type, yz_val **v, int vlen);
int asf_func_def(const char *name, int len, enum YZ_TYPE type);
int asf_func_ret(yz_val *v, int is_main);

backend_scope_status *asf_scope_begin();
int asf_scope_end(backend_scope_status *raw_status);

int asf_op_add(struct expr *e);
int asf_op_div(struct expr *e);
int asf_op_mul(struct expr *e);
int asf_op_sub(struct expr *e);
int asf_op_and(struct expr *e);
int asf_op_eq(struct expr *e);
int asf_op_ge(struct expr *e);
int asf_op_gt(struct expr *e);
int asf_op_le(struct expr *e);
int asf_op_lt(struct expr *e);
int asf_op_ne(struct expr *e);
int asf_op_not(struct expr *e);
int asf_op_or(struct expr *e);

int asf_op_assign(struct expr *e);
int asf_op_assign_add(struct expr *e);
int asf_op_assign_div(struct expr *e);
int asf_op_assign_mul(struct expr *e);
int asf_op_assign_sub(struct expr *e);

int asf_var_set(char *name, yz_val *val);
int asf_var_immut_init(char *name, yz_val *val);

int asf_while_begin(backend_scope_status *raw_status);
int asf_while_end(backend_scope_status *raw_status);

#endif
