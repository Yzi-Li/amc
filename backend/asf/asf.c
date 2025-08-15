/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/file.h"
#include "include/register.h"
#include "../../include/backend.h"
#include "../../include/backend/object.h"
#include <string.h>

struct backend backend_asf = {
	.end             = asf_end,
	.file_end        = asf_file_end,
	.file_get_suffix = asf_file_get_suffix,
	.file_new        = asf_file_new,
	.init            = asf_init,
	.stop            = asf_stop,

	.array_def      = asf_array_def,
	.array_set_elem = asf_array_set_elem,

	.cond_elif     = asf_cond_elif,
	.cond_else     = asf_cond_else,
	.cond_if       = asf_cond_if,
	.cond_if_begin = asf_cond_if_begin,

	.const_def_str = asf_const_def_str,

	.dec_c_fn    = asf_dec_c_fn,
	.dec_syscall = asf_dec_syscall,

	.func_call    = asf_func_call,
	.func_def     = asf_func_def,
	.func_def_end = asf_func_def_end,
	.func_ret     = asf_func_ret,

	.null_handle_begin = asf_null_handle_begin,
	.null_handle_end   = asf_null_handle_end,

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

	.struct_def               = asf_struct_def,
	.struct_set_elem          = asf_struct_set_elem,
	.struct_set_elem_from_ptr = asf_struct_set_elem_from_ptr,

	.symbol_get_path    = asf_symbol_get_path,
	.symbol_status_free = asf_symbol_status_free,

	.var_immut_init = asf_var_immut_init,
	.var_set        = asf_var_set,

	.while_begin = asf_while_begin,
	.while_cond  = asf_while_cond,
	.while_end   = asf_while_end
};

int asf_init(int argc, char *argv[])
{
	return asf_regs_init();
}

int asf_stop(enum BE_STOP_SIGNAL bess)
{
	switch (bess) {
	default:
		object_head_free(cur_obj);
		break;
	}
	return 0;
}

int asf_end(str *output)
{
	if (asf_link_files(output))
		return 1;
	return 0;
}
