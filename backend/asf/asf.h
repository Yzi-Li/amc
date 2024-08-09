#ifndef AMC_BE_ASF_H
#define AMC_BE_ASF_H
#include "../../include/backend.h"

enum OBJ_SECTION_TYPE {
	ASF_OBJ_TEXT,
	ASF_OBJ_DATA,
	ASF_OBJ_RODATA
};

int asf_end();
int asf_file_new(struct file *f);
int asf_init(int argc, char *argv[]);
int asf_stop(enum BE_STOP_SIGNAL bess);

int asf_func_call(const char *name, yz_val **v, int vlen);
int asf_func_def(const char *name, int len, enum YZ_TYPE type);
int asf_func_ret(yz_val *v);
int asf_op_add(yz_val *l, yz_val *r);
int asf_op_div(yz_val *l, yz_val *r);
int asf_op_mul(yz_val *l, yz_val *r);
int asf_op_sub(yz_val *l, yz_val *r);
int asf_op_and(yz_val *l, yz_val *r);
int asf_op_eq(yz_val *l, yz_val *r);
int asf_op_ge(yz_val *l, yz_val *r);
int asf_op_gt(yz_val *l, yz_val *r);
int asf_op_le(yz_val *l, yz_val *r);
int asf_op_lt(yz_val *l, yz_val *r);
int asf_op_ne(yz_val *l, yz_val *r);
int asf_op_not(yz_val *l, yz_val *r);
int asf_op_or(yz_val *l, yz_val *r);
int asf_op_assignment(yz_val *l, yz_val *r);
int asf_var_set(str *name, yz_val *val);
int asf_var_immut_set(str *name, yz_val *val);

static struct backend backend_asf = {
	.end      = asf_end,
	.file_new = asf_file_new,
	.init     = asf_init,
	.stop     = asf_stop,

	.func_call = asf_func_call,
	.func_def  = asf_func_def,
	.func_ret  = asf_func_ret,

	.ops = {
		[OP_ADD] = asf_op_add,
		[OP_AND] = asf_op_and,
		[OP_DIV] = asf_op_div,
		[OP_EQ]  = asf_op_eq,
		[OP_GE]  = asf_op_ge,
		[OP_GT]  = asf_op_gt,
		[OP_LE]  = asf_op_le,
		[OP_LT]  = asf_op_lt,
		[OP_MUL] = asf_op_mul,
		[OP_NE]  = asf_op_ne,
		[OP_NOT] = asf_op_not,
		[OP_OR]  = asf_op_or,
		[OP_SUB] = asf_op_sub,

		[OP_ASSIGNMENT] = asf_op_assignment
	},

	.var_set       = asf_var_set,
	.var_immut_set = asf_var_immut_set
};

#endif
