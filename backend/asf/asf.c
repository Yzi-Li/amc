#include "include/asf.h"
#include "include/register.h"
#include "../../include/backend.h"
#include "../../include/backend/target.h"
#include <stdlib.h>
#include <string.h>

struct backend backend_asf = {
	.end      = asf_end,
	.file_new = asf_file_new,
	.init     = asf_init,
	.stop     = asf_stop,

	.array_def      = asf_array_def,
	.array_set_elem = asf_array_set_elem,

	.cond_elif = asf_cond_elif,
	.cond_else = asf_cond_else,
	.cond_if   = asf_cond_if,

	.const_def_str = asf_const_def_str,

	.func_call = asf_func_call,
	.func_def  = asf_func_def,
	.func_ret  = asf_func_ret,

	.ops = {
		[OP_ADD] = asf_op_add,
		[OP_DIV] = asf_op_div,
		[OP_MUL] = asf_op_mul,
		[OP_SUB] = asf_op_sub,

		[OP_EQ]  = asf_op_eq,
		[OP_NE]  = asf_op_ne,
		[OP_LE]  = asf_op_le,
		[OP_LT]  = asf_op_lt,
		[OP_GE]  = asf_op_ge,
		[OP_GT]  = asf_op_gt,

		[OP_AND] = asf_op_and,
		[OP_OR]  = asf_op_or,
		[OP_NOT] = asf_op_not,

		[OP_ASSIGN] = NULL,
		[OP_ASSIGN_ADD] = NULL,
		[OP_ASSIGN_DIV] = NULL,
		[OP_ASSIGN_MUL] = NULL,
		[OP_ASSIGN_SUB] = NULL,

		[OP_EXTRACT_VAL] = asf_op_extract_val,
		[OP_GET_ADDR] = asf_op_get_addr
	},

	.scope_begin = asf_scope_begin,
	.scope_end   = asf_scope_end,

	.struct_def      = asf_struct_def,
	.struct_set_elem = asf_struct_set_elem,

	.symbol_status_free = asf_symbol_status_free,
	.syscall = asf_syscall,

	.var_immut_init = asf_var_immut_init,
	.var_set        = asf_var_set,

	.while_begin = asf_while_begin,
	.while_end   = asf_while_end
};

int asf_init(int argc, char *argv[])
{
	objs = malloc(sizeof(void*));
	return asf_regs_init();
}

int asf_file_new(struct file *f)
{
	const char *temp_rodata = ".section .rodata\n";
	struct object_node *rodata = NULL;
	cur_obj++;
	objs[cur_obj] = calloc(3, sizeof(struct object_head));
	rodata = malloc(sizeof(*rodata));
	if (object_append(&objs[cur_obj][ASF_OBJ_RODATA], rodata))
		goto err_free_rodata;
	rodata->s = str_new();
	str_append(rodata->s, strlen(temp_rodata), temp_rodata);
	return 0;
err_free_rodata:
	free(rodata);
	return 1;
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
	return target_write(objs, ASF_OBJ_COUNT);
}
