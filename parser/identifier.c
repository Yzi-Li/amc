#include "include/array.h"
#include "include/expr.h"
#include "include/identifier.h"
#include "include/keywords.h"
#include "include/struct.h"
#include "include/token.h"
#include "include/type.h"
#include "include/utils.h"
#include "../include/backend.h"
#include "../include/comptime/mut.h"
#include "../include/comptime/type.h"
#include "../include/comptime/val.h"
#include "../include/expr.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../utils/str/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int identifier_assign_backend_call(struct symbol *sym, yz_val *val,
		enum OP_ID mode);
static int identifier_handle_val_type(yz_val *src, yz_val *dest);
static int let_init_constructor(struct file *f, struct symbol *sym,
		struct scope *scope);
static int let_init_val(struct file *f, struct symbol *sym,
		struct scope *scope);
static int let_reg_sym(struct file *f, struct symbol *sym,
		struct scope *scope);

int identifier_assign_backend_call(struct symbol *sym, yz_val *val,
		enum OP_ID mode)
{
	if (sym->flags.mut) {
		if (backend_call(var_set)(sym, mode, val))
			return 1;
	} else {
		if (mode != OP_ASSIGN)
			goto err_unsupport_op;
		if (backend_call(var_immut_init)(sym, val))
			return 1;
	}
	sym->flags.is_init = 1;
	return 0;
err_unsupport_op:
	printf("amc: identifier_assign_backend_call: "
			"Unsupport operator for immutable identifier!\n");
	return 1;
}

int identifier_handle_val_type(yz_val *src, yz_val *dest)
{
	if (comptime_type_check_equal(src, dest))
		return 1;
	if (!YZ_IS_DIGIT(src->type) || !YZ_IS_DIGIT(dest->type))
		return 0;
	src->type = dest->type;
	return 0;
}

int let_init_constructor(struct file *f, struct symbol *sym,
		struct scope *scope)
{
	file_pos_next(f);
	file_skip_space(f);
	if (f->src[f->pos] == '\n')
		file_line_next(f);
	switch (sym->result_type.type) {
	case YZ_ARRAY:
		return constructor_array(f, sym, scope);
		break;
	case YZ_STRUCT:
		return constructor_struct(f, sym, scope);
		break;
	default:
		return 1;
	}
	return 0;
}

int let_init_val(struct file *f, struct symbol *sym, struct scope *scope)
{
	file_pos_next(f);
	file_skip_space(f);
	if (f->src[f->pos] == '{')
		return let_init_constructor(f, sym, scope);
	if (identifier_assign_val(f, sym, OP_ASSIGN, scope))
		return 1;
	return keyword_end(f);
}

int let_reg_sym(struct file *f, struct symbol *sym, struct scope *scope)
{
	sym->argc = 1;
	if (symbol_register(sym, &scope->sym_groups[SYMG_SYM]))
		goto err_cannot_register_sym;
	return 0;
err_cannot_register_sym:
	printf("amc: let_reg_sym: %lld,%lld: Cannot register symbol!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_let(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct symbol *result = calloc(1, sizeof(*result));
	result->type = SYM_IDENTIFIER;
	if (parse_type_name_pair(f, result, scope))
		goto err_free_result;
	if (let_reg_sym(f, result, scope))
		goto err_free_result;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	if (parse_comment(f))
		return 0;
	if (f->src[f->pos] != '=')
		goto err_syntax_err;
	return let_init_val(f, result, scope);
err_syntax_err:
	printf("amc: parse_let: %lld,%lld: Syntax error!\n",
			f->cur_line, f->cur_column);
err_free_result:
	free_symbol(result);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int identifier_assign_get_val(struct file *f, struct scope *scope,
		yz_val *dest_type, yz_val **result)
{
	struct expr *expr = NULL;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		return err_print_pos(__func__, "Cannot parse expr!",
				orig_line, orig_column);
	if (expr_apply(expr, scope) > 0)
		goto err_cannot_apply_expr;
	if ((*result = identifier_expr_val_handle(&expr, dest_type))
			== NULL)
		goto err_cannot_handle_expr;
	return 0;
err_cannot_apply_expr:
	free_expr(expr);
	return err_print_pos(__func__, "Cannot apply expr!",
			orig_line, orig_column);
err_cannot_handle_expr:
	free_expr(expr);
	return err_print_pos(__func__, "Cannot handle expr value!",
			orig_line, orig_column);
}

int identifier_assign_val(struct file *f, struct symbol *sym, enum OP_ID mode,
		struct scope *scope)
{
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	yz_val *val = NULL;
	sym->flags.comptime_flag.checked_null = 0;
	if (!comptime_check_sym_can_assign(sym))
		return err_print_pos(__func__, NULL, orig_line, orig_column);
	if (identifier_assign_get_val(f, scope, &sym->result_type, &val))
		return 1;
	if (!comptime_check_sym_can_assign_val(sym, val))
		return err_print_pos(__func__, NULL, orig_line, orig_column);
	if (identifier_assign_backend_call(sym, val, mode))
		return err_print_pos(__func__, "Backend call failed!",
				orig_line, orig_column);
	free_yz_val(val);
	if (f->src[f->pos] == ']')
		file_pos_next(f);
	return 0;
}

yz_val *identifier_expr_val_handle(struct expr **e, yz_val *type)
{
	yz_val *val = NULL;
	if ((*e)->op == NULL && (*e)->valr == NULL) {
		val = (*e)->vall;
		if (identifier_handle_val_type(val, type))
			goto err_get_type;
		return val;
	}
	val = calloc(1, sizeof(*val));
	val->type = AMC_EXPR;
	val->v = *e;
	if (identifier_handle_val_type(val, type))
		goto err_get_type;
	return val;
err_get_type:
	return NULL;
}

int identifier_read(struct file *f, yz_val *val, struct scope *scope)
{
	char *err_msg;
	struct symbol *sym = NULL;
	str token = TOKEN_NEW;
	if (token_read_before(SPECIAL_TOKEN_END, &token, f) == NULL)
		return 1;
	if (!symbol_find_in_group_in_scope(&token, &sym, scope, SYMG_SYM))
		goto err_identifier_not_found;
	val->type = AMC_SYM;
	val->v = sym;
	if (f->src[f->pos] == '[')
		return array_get_elem(f, val, scope);
	if (f->src[f->pos] == '.') {
		if (sym->result_type.type != YZ_STRUCT)
			goto err_syntax_err;
		return struct_get_elem(f, val, scope);
	}
	file_skip_space(f);
	return 0;
err_identifier_not_found:
	err_msg = str2chr(token.s, token.len);
	printf("amc: identifier_read: %lld,%lld: "
			"Identifier: '%s' not found!\n",
			f->cur_line, f->cur_column,
			err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_syntax_err:
	err_msg = str2chr(token.s, token.len);
	printf("amc: identifier_read: %lld,%lld: "
			"Identifier: '%s' not struct!\n",
			f->cur_line, f->cur_column,
			err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}
