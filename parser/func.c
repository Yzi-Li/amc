#include "include/expr.h"
#include "include/block.h"
#include "include/keywords.h"
#include "include/type.h"
#include "../include/backend.h"
#include "../include/file.h"
#include "../include/parser.h"
#include "../include/scope.h"
#include "../include/token.h"
#include "../utils/utils.h"
#include <stdio.h>

struct func_call_handle {
	struct file *f;
	struct symbol *fn;
	int index;
	struct scope *scope;
	yz_val **vals;
};

static yz_val *func_call_arg_handle(struct expr *expr, yz_val *arg);
static yz_val *func_call_arg_val_get(struct expr *expr, yz_val *arg);
static int func_call_main(struct file *f, struct symbol *sym,
		struct scope *scope);
static int func_call_read_arg(const char *se, struct file *f, void *data);
static yz_val **func_call_read_args(struct file *f, struct symbol *fn,
		struct scope *scope);
static int func_def_block_start(struct file *f);
static int func_def_check_main(const char *name, int len);
static int func_def_end_scope(struct symbol *fn, struct scope *scope);
static int func_def_main(struct file *f, struct scope *fn_scope);
static int func_def_read_arg(const char *se, struct file *f, void *data);
static int func_def_read_arg_name(const char *se, struct file *f, str *name);
static int func_def_read_args(struct file *f, struct symbol *fn,
		struct scope *scope);
static int func_def_read_block(struct file *f, struct scope *scope);
static int func_def_read_name(struct file *f, struct symbol *fn);
static int func_def_read_type(struct file *f, yz_val *type);
static int func_ret_get_val(yz_val *val, yz_val *fn_result, struct expr *expr);

yz_val *func_call_arg_handle(struct expr *expr, yz_val *arg)
{
	yz_val *type = NULL,
	       *val = func_call_arg_val_get(expr, arg);
	if ((type = yz_type_max(val, arg)) == NULL)
		goto err_wrong_arg_type;
	return val;
err_wrong_arg_type:
	printf("amc: func_call_read_arg: Wrong argument type\n"
			"| Val type: \"%s\"\n"
			"| Arg type: \"%s\"\n",
			yz_get_type_name(val),
			yz_get_type_name(arg));
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
}

yz_val *func_call_arg_val_get(struct expr *expr, yz_val *arg)
{
	yz_val *result = NULL;
	if (expr->op == NULL && expr->valr == NULL)
		return expr->vall;
	result = malloc(sizeof(*result));
	result->type = AMC_EXPR;
	result->v = expr;
	return result;
}

int func_call_main(struct file *f, struct symbol *sym, struct scope *scope)
{
	printf("amc: func_call_main: %lld,%lld: "
			"You cannot call the main function!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_call_read_arg(const char *se, struct file *f, void *data)
{
	struct expr *expr = NULL;
	struct func_call_handle *handle = data;
	yz_val *result = NULL;
	if (handle->index > handle->fn->argc - 1)
		goto err_too_many_args;
	if ((expr = parse_expr(f, 1, handle->scope)) == NULL)
		goto err_cannot_parse_arg;
	if (expr_apply(expr, handle->scope) > 0)
		goto err_cannot_parse_arg;
	if ((result = func_call_arg_handle(expr,
			handle->fn->args[handle->index])) == NULL)
		goto err_cannot_handle_arg;
	handle->vals[handle->index] = result;
	handle->index += 1;
	return token_list_elem_end(',', f);
err_too_many_args:
	printf("amc: func_call_read_arg: %lld,%lld: Too many parameters.\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_parse_arg:
	printf("amc: func_call_read_arg: %lld,%lld: Cannot parse argument.\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_handle_arg:
	printf("|< amc: func_call_read_arg: %lld,%lld: "
			"Cannot handle argument.\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

yz_val **func_call_read_args(struct file *f, struct symbol *fn,
		struct scope *scope)
{
	char *err_msg;
	yz_val **result = calloc(fn->argc, sizeof(*result));
	struct func_call_handle *handle = malloc(sizeof(*handle));
	handle->f = f;
	handle->fn = fn;
	handle->index = 0;
	handle->scope = scope;
	handle->vals = result;
	if (token_parse_list(",]", handle, f, func_call_read_arg))
		goto err_free_result;
	if (handle->index < fn->argc)
		goto err_too_few_arg;
	free_safe(handle);
	if (f->src[f->pos] != ']')
		return NULL;
	file_pos_next(f);
	file_skip_space(f);
	return result;
err_free_result:
	free_safe(handle);
	for (int i = 0; i < fn->argc; i++)
		free_safe(result[i]);
	free_safe(result);
	return NULL;
err_too_few_arg:
	err_msg = str2chr(fn->name, fn->name_len);
	printf("amc: func_call_read_args: %lld,%lld: Too few arguments!\n"
			"| Function: \"%s\"\n"
			"| Need %d but only has %d\n",
			f->cur_line, f->cur_column,
			err_msg,
			fn->argc, handle->index);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	goto err_free_result;
}

int func_def_block_start(struct file *f)
{
	char *err_msg;
	str token = TOKEN_NEW;
	if (token_next(&token, f))
		return 1;
	if (token.s[token.len - 1] == ';') {
		token.len -= 1;
		file_line_next(f);
	}
	if (token.len != 2)
		goto err_not_func_def_start;
	if (token.s[0] != '=' || token.s[1] != '>')
		goto err_not_func_def_start;
	parse_comment(f);
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	return 0;
err_not_func_def_start:
	err_msg = str2chr(token.s, token.len);
	printf("amc: func_def_block_start: %lld,%lld:"
			"Function define start character not found\n"
			"| Token: \"%s\"\n"
			"|         ^\n",
			f->cur_line, f->cur_column,
			err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_check_main(const char *name, int len)
{
	if (len != 4)
		return 0;
	if (strncmp(name, "main", len) == 0)
		return 1;
	return 0;
}

int func_def_end_scope(struct symbol *fn, struct scope *scope)
{
	for (int i = 0; i < fn->argc; i++)
		scope->sym_groups[SYMG_SYM].symbols[i] = NULL;
	scope_end(scope);
	return 0;
}

int func_def_main(struct file *f, struct scope *fn_scope)
{
	fn_scope->fn->result_type.type = YZ_I8;
	fn_scope->fn->result_type.v = NULL;
	parser_global_conf.has_main = 1;
	while (f->src[f->pos] != '=')
		file_pos_next(f);
	if (backend_call(func_def)("_start", 7, NULL))
		goto err_free_fn;
	fn_scope->fn->parse_function = func_call_main;
	if (symbol_register(fn_scope->fn,
				&fn_scope->parent->sym_groups[SYMG_FUNC]))
		goto err_free_fn;
	return func_def_read_block(f, fn_scope);
err_free_fn:
	free_safe(fn_scope->fn);
	return 1;
}

int func_def_read_arg(const char *se, struct file *f, void *data)
{
	str name = TOKEN_NEW;
	struct symbol *sym = NULL;
	struct scope *scope = data;
	if (func_def_read_arg_name(se, f, &name))
		return 1;
	sym = calloc(1, sizeof(*sym));
	if (func_def_read_type(f, &sym->result_type))
		goto err_unsupport_type;
	sym->name = name.s;
	sym->name_len = name.len;
	sym->parse_function = NULL;
	sym->argc = 2 + scope->fn->argc;
	sym->args = NULL;
	if (symbol_register(sym, &scope->sym_groups[SYMG_SYM]))
		goto err_free_sym;
	if (symbol_args_append(scope->fn, &sym->result_type))
		goto err_free_sym;
	return token_list_elem_end(',', f);
err_unsupport_type:
	printf("amc: func_def_reg_arg: Unsupport type!\n");
err_free_sym:
	free_safe(sym);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_read_arg_name(const char *se, struct file *f, str *name)
{
	char *end = NULL;
	if ((end = token_read_before(" :\n\t,)", name, f)) == NULL)
		return 1;
	file_skip_space(f);
	keyword_end(f);
	file_skip_space(f);
	if (f->src[f->pos] != ':')
		return 1;
	return 0;
}

int func_def_read_args(struct file *f, struct symbol *fn, struct scope *scope)
{
	if (f->src[f->pos] != '(')
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	if (token_parse_list(",)", scope, f, func_def_read_arg))
		goto err_args_cannot_parse;
	if (f->src[f->pos] != ')')
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	return 0;
err_args_cannot_parse:
	printf("amc: parse_func_def: Cannot parse function arguments!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_read_block(struct file *f, struct scope *scope)
{
	if (func_def_block_start(f))
		return 1;
	return parse_block(f, scope);
}

int func_def_read_name(struct file *f, struct symbol *fn)
{
	int len = 0;
	fn->name = &f->src[f->pos];
	for (; f->src[f->pos] != ' '
			&& f->src[f->pos] != '\t'
			&& f->src[f->pos] != '\n'
			&& f->src[f->pos] != '('
			&& f->src[f->pos] != ':'; len++) {
		if (f->src[f->pos] == '\0')
			goto err_eof;
		file_pos_next(f);
	}
	file_skip_space(f);
	fn->name_len = len;
	return 0;
err_eof:
	printf("amc: func_def_read_name: end of file\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_read_type(struct file *f, yz_val *type)
{
	if (f->src[f->pos] != ':')
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	if (parse_type(f, type))
		goto err_cannot_get_type;
	return keyword_end(f);
err_cannot_get_type:
	printf("amc: func_def_read_type: Cannot get type!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_ret_get_val(yz_val *val, yz_val *fn_result, struct expr *expr)
{
	int is_single = EXPR_IS_SINGLE_TERM(expr);
	val->type = *expr->sum_type;
	if (is_single) {
		val->l = expr->vall->l;
		val->type = expr->vall->type;
	} else {
		val->v = expr;
		val->type = AMC_EXPR;
	}
	if (yz_type_max(val, fn_result) == NULL)
		goto err_type;
	if (is_single) {
		if (val->type == AMC_SYM)
			return 0;
		val->type = fn_result->type;
		return 0;
	}
	*expr->sum_type = fn_result->type;
	return 0;
err_type:
	printf("amc: func_ret_get_val: Type error: \"%s\"!\n",
			yz_get_type_name(val));
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_func_call(struct file *f, struct symbol *sym, struct scope *scope)
{
	yz_val **v = NULL;
	const char *name = NULL;
	v = func_call_read_args(f, sym, scope);
	if (v == NULL)
		return 1;
	if (sym->name[sym->name_len - 1] != '\0') {
		name = str2chr(sym->name, sym->name_len);
	} else {
		name = sym->name;
	}
	return backend_call(func_call)(name, &sym->result_type, v, sym->argc);
}

int parse_func_def(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct symbol *result = calloc(1, sizeof(*result));
	struct scope fn_scope = {
		.fn = result,
		.indent = scope->indent,
		.parent = scope,
		.status = backend_call(scope_begin)(),
		.status_type = SCOPE_IN_BLOCK,
		.sym_groups = {}
	};
	if (scope_check_is_correct(&fn_scope))
		return 1;
	if (func_def_read_name(f, result))
		goto err_free_result;
	if (func_def_check_main(result->name, result->name_len))
		return func_def_main(f, &fn_scope);
	if (f->src[f->pos] != ':' && func_def_read_args(f, result, &fn_scope))
		goto err_free_result;
	if (func_def_read_type(f, &result->result_type))
		goto err_free_result;
	result->parse_function = parse_func_call;
	if (backend_call(func_def)(result->name,
			result->name_len,
			&result->result_type))
		goto err_free_result;
	result->flags.in_block = 1;
	if (symbol_register(result, &scope->sym_groups[SYMG_FUNC]))
		goto err_free_result;
	if (func_def_read_block(f, &fn_scope))
		goto err_free_result;
	return func_def_end_scope(result, &fn_scope);
err_free_result:
	free_safe(result);
	return 1;
}

int parse_func_ret(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct expr *expr = NULL;
	int is_main = 0, ret = 0;
	yz_val val = {};
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		return 1;
	if ((ret = expr_apply(expr, scope)) > 0)
		goto err_free_expr;
	if (f->src[f->pos] == '\n')
		file_line_next(f);
	parse_comment(f);
	is_main = strncmp(scope->fn->name, "main", 4) == 0;
	if (func_ret_get_val(&val, &scope->fn->result_type, expr))
		goto err_get_val_failed;
	if (backend_call(func_ret)(&val, is_main))
		goto err_backend_failed;
	expr_free(expr);
	return 0;
err_free_expr:
	expr_free(expr);
	return 1;
err_get_val_failed:
	printf("amc: parse_func_ret: %lld,%lld: Get value failed!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	goto err_free_expr;
err_backend_failed:
	printf("amc: parse_func_ret: Backend call failed!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	goto err_free_expr;
}
