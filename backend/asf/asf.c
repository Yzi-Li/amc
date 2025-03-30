#include "asf.h"
#include "register.h"
#include "stack.h"
#include "../../include/backend.h"
#include "../../include/backend/target.h"

struct backend backend_asf = {
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
	},

	.var_set        = asf_var_set,
	.var_immut_init = asf_var_immut_init
};

int asf_init(int argc, char *argv[])
{
	objs = malloc(sizeof(void*));
	return asf_regs_init();
}

int asf_file_new(struct file *f)
{
	cur_obj++;
	objs[cur_obj] = calloc(3, sizeof(void*));
	return 0;
}

int asf_stop(enum BE_STOP_SIGNAL bess)
{
	switch (bess) {
	case BE_STOP_SIGNAL_NULL:
		//break;
	case BE_STOP_SIGNAL_ERR:
	default:
		objects_free(objs[cur_obj]);
		break;
	}

	return 0;
}

int asf_end()
{
	return target_write(objs, 1);
}
