/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_H
#define AMC_BE_ASF_H
#include "../../../include/backend.h"

enum OBJ_SECTION_TYPE {
	ASF_OBJ_TEXT,
	ASF_OBJ_DATA,
	ASF_OBJ_RODATA,

	ASF_OBJ_COUNT
};

int asf_end(str *output);
int asf_file_end(const char *path, int path_len);
char *asf_file_get_suffix(int *result_len, int *need_free);
int asf_file_new(struct file *f);
int asf_init(int argc, char *argv[]);
int asf_stop(enum BE_STOP_SIGNAL bess);

int asf_array_def(backend_symbol_status **raw_sym_stat, yz_val **vs, int len);
int asf_array_set_elem(struct symbol *sym, yz_val *offset, yz_val *val,
		enum OP_ID mode);

int asf_cond_elif(backend_scope_status *raw_status);
int asf_cond_else(backend_scope_status *raw_status);
int asf_cond_if(backend_scope_status *raw_status);
int asf_cond_if_begin(backend_scope_status *raw_status);

int asf_const_def_str(backend_const *self, str *s);

int asf_dec_c_fn(str *name, int argc);
int asf_dec_syscall(int code, int argc);

int asf_func_call(struct symbol *fn, yz_val **v, int vlen);
backend_func_def_handle *asf_func_def(struct symbol *fn, int pub, int main);
int asf_func_def_end(backend_func_def_handle *handle);
int asf_func_ret(yz_val *v, int is_main);

int asf_null_handle_begin(backend_null_handle **handle, yz_val *val);
int asf_null_handle_end(backend_null_handle *handle);

backend_scope_status *asf_scope_begin();
int asf_scope_end(backend_scope_status *raw_status);

int asf_struct_def(backend_symbol_status *raw_sym_stat, yz_val **vs, int len);
int asf_struct_set_elem(struct symbol *sym, int index, yz_val *val,
		enum OP_ID mode);

int asf_symbol_get_path(str *result, str *mod, const char *name, int name_len);
void asf_symbol_status_free(backend_symbol_status *raw_stat);

int asf_op_add(struct expr *e);
int asf_op_div(struct expr *e);
int asf_op_mul(struct expr *e);
int asf_op_sub(struct expr *e);

int asf_op_eq(struct expr *e);
int asf_op_ne(struct expr *e);
int asf_op_le(struct expr *e);
int asf_op_lt(struct expr *e);
int asf_op_ge(struct expr *e);
int asf_op_gt(struct expr *e);

int asf_op_and(struct expr *e);
int asf_op_or(struct expr *e);
int asf_op_not(struct expr *e);

int asf_op_extract_val(struct expr *e);
int asf_op_get_addr(struct expr *e);

int asf_var_immut_init(struct symbol *sym, yz_val *val);
int asf_var_set(struct symbol *sym, enum OP_ID mode, yz_val *val);

int asf_while_begin(backend_scope_status *raw_status);
int asf_while_cond(backend_scope_status *raw_status);
int asf_while_end(backend_scope_status *raw_status);

#endif
