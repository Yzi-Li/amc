/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/block.h"
#include "include/expr.h"
#include "include/identifier.h"
#include "include/keywords.h"
#include "include/token.h"
#include "include/type.h"
#include "../include/backend.h"
#include "../include/file.h"
#include "../include/parser.h"
#include "../include/scope.h"
#include "../include/token.h"
#include "../include/comptime/ptr.h"
#include "../utils/utils.h"
#include "include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct func_call_handle {
	struct file *f;
	struct symbol *fn;
	int index;
	struct parser *parser;
	yz_val **vals;
};

static yz_val *func_call_arg_handle(struct expr *expr, struct symbol *arg);
static int func_call_main(struct parser *parser);
static int func_call_read_arg(const char *se, struct file *f, void *data);
static yz_val **func_call_read_args(struct parser *parser);
static int func_call_read_callee(struct parser *parser,
		struct symbol **callee);
static int func_call_read_token(struct file *f, str *token);
static int func_def_block_start(struct parser *parser);
static int func_def_check_main(const char *name, int len);
static int func_def_end_scope(struct parser *parser);
static int func_def_inherit_decorators(struct decorators *src,
		struct symbol *dest);
static int func_def_main(struct parser *parser);
static int func_def_read_arg(const char *se, struct file *f, void *data);
static int func_def_read_args(struct parser *parser);
static int func_def_read_name(struct parser *parser);
static int func_def_read_type(struct parser *parser);
static yz_val *func_ret_get_val(struct symbol *fn, struct expr *expr);

yz_val *func_call_arg_handle(struct expr *expr, struct symbol *arg)
{
	yz_val *result = expr2yz_val(expr);
	if ((result->type.type == AMC_SYM || result->type.type == YZ_NULL)
			&& arg->result_type.type == YZ_PTR) {
		if (!comptime_ptr_check_can_null(result, arg))
			goto err_free_result;
	}
	if (identifier_handle_val_type(&result->type, &arg->result_type))
		goto err_free_result;
	return result;
err_free_result:
	free_yz_val(result);
	return NULL;
}

int func_call_main(struct parser *parser)
{
	printf("amc: func_call_main: %lld,%lld: "
			"You cannot call the main function!\n",
			parser->f->cur_line, parser->f->cur_column);
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
	if ((expr = parse_expr(handle->parser, 1)) == NULL)
		goto err_print_pos;
	if (expr_apply(handle->parser, expr) > 0)
		goto err_print_pos;
	if ((result = func_call_arg_handle(expr,
			handle->fn->args[handle->index])) == NULL)
		goto err_print_pos;
	handle->vals[handle->index] = result;
	handle->index += 1;
	return token_list_elem_end(',', f);
err_too_many_args:
	printf("amc: func_call_read_arg: %lld,%lld: Too many parameters.\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_print_pos:
	return err_print_pos(__func__, NULL,
			f->cur_line, f->cur_column);
}

yz_val **func_call_read_args(struct parser *parser)
{
	yz_val **result = calloc(parser->sym->argc, sizeof(*result));
	struct func_call_handle *handle = malloc(sizeof(*handle));
	handle->f = parser->f;
	handle->fn = parser->sym;
	handle->index = 0;
	handle->parser = parser;
	handle->vals = result;
	if (token_parse_list(",]", handle, parser->f, func_call_read_arg))
		goto err_free_result;
	if (handle->index < parser->sym->argc)
		goto err_too_few_arg;
	free_safe(handle);
	if (parser->f->src[parser->f->pos] != ']')
		return NULL;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	return result;
err_free_result:
	free_safe(handle);
	for (int i = 0; i < parser->sym->argc; i++)
		free_safe(result[i]);
	free_safe(result);
	return NULL;
err_too_few_arg:
	printf("amc: func_call_read_args: %lld,%lld: Too few arguments!\n"
			"| Function: \"%s\"\n"
			"| Need %d but only has %d\n",
			parser->f->cur_line, parser->f->cur_column,
			parser->sym->name.s,
			parser->sym->argc, handle->index);
	backend_stop(BE_STOP_SIGNAL_ERR);
	goto err_free_result;
}

int func_call_read_callee(struct parser *parser, struct symbol **callee)
{
	char *err_msg = NULL;
	yz_module *mod = NULL;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	int ret = 0;
	struct symbol *result = NULL;
	struct scope *scope = parser->scope;
	str token = TOKEN_NEW;
	if ((ret = func_call_read_token(parser->f, &token)) > 0)
		return 1;
	if (parser->f->src[parser->f->pos] == '.') {
		if ((mod = parser_imported_find(&parser->imported, &token))
				== NULL)
			return 1;
		token.len = 0;
		if ((ret = func_call_read_token(parser->f, &token)) > 0)
			return 1;
		scope = mod->scope;
	}
	if (!symbol_find(&token, &result, scope, SYMG_FUNC))
		goto err_func_not_found;
	if (!result->flags.in_block)
		goto err_func_not_in_block;
	if (ret == -1 && result->argc > 0)
		goto err_func_miss_args;
	*callee = result;
	return 0;
err_func_not_found:
	err_msg = str2chr(token.s, token.len);
	printf("amc: func_call_read: %lld,%lld: Function: '%s' not found!\n",
			orig_line, orig_column, err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_func_not_in_block:
	printf("amc: func_call_read: %lld,%lld: "
			"Function: '%s' cannot be called in block!\n",
			orig_line, orig_column, result->name.s);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_func_miss_args:
	printf("amc: func_call_read: %lld,%lld: "
			"Function: '%s' need %d arguments "
			"but not have any arguments!\n",
			orig_line, orig_column,
			result->name.s, result->argc);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_call_read_token(struct file *f, str *token)
{
	file_pos_next(f);
	file_skip_space(f);
	if (token_read_before(SPECIAL_TOKEN_END, token, f) == NULL)
		return 1;
	if (f->src[f->pos] == ']')
		return -1;
	file_skip_space(f);
	return keyword_end(f);
}

int func_def_block_start(struct parser *parser)
{
	str block_start = {.len = 2, .s = "=>"};
	if (try_next_line(parser->f)) {
		parser->sym->flags.only_declaration = 1;
		return -1;
	}
	if (!token_try_read(&block_start, parser->f))
		goto err_not_func_def_start;
	return 0;
err_not_func_def_start:
	printf("amc: func_def_block_start: %lld,%lld: "
			"Function: '%s' define start character not found\n",
			parser->f->cur_line, parser->f->cur_column,
			parser->sym->name.s);
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

int func_def_end_scope(struct parser *parser)
{
	struct scope *parent = parser->scope->parent;
	for (int i = 0; i < parser->scope->fn->argc; i++)
		parser->scope->sym_groups[SYMG_SYM].symbols[i] = NULL;
	scope_end(parser->scope);
	parser->scope = parent;
	return 0;
}

int func_def_inherit_decorators(struct decorators *src,
		struct symbol *dest)
{
	if ((dest->hooks = hooks_inherit(&src->hooks)) == NULL)
		return 1;
	src->used = 1;
	return 0;
}

int func_def_main(struct parser *parser)
{
	struct symbol *fn = parser->scope->fn;
	if (!parser->stat.has_pub)
		goto err_not_pub;
	fn->result_type.type = YZ_I8;
	fn->result_type.v = NULL;
	global_parser.has_main = 1;
	while (parser->f->src[parser->f->pos] != '\n'
			&& parser->f->src[parser->f->pos] != ';')
		file_pos_next(parser->f);
	if (backend_call(func_def)(fn, 1, 1))
		goto err_free_fn;
	fn->parse_function = func_call_main;
	if (symbol_register(fn, &parser->scope_pub->sym_groups[SYMG_FUNC]))
		goto err_free_fn;
	return parse_block(parser);
err_not_pub:
	printf("amc: func_def_main: "ERROR_STR":\n"
			"| Function: 'main' must be declared as 'pub'.\n");
	return 1;
err_free_fn:
	free_symbol(parser->scope->fn);
	return 1;
}

int func_def_read_arg(const char *se, struct file *f, void *data)
{
	struct parser *parser = data;
	int ret = 0;
	struct symbol *sym = NULL;
	sym = calloc(1, sizeof(*sym));
	if ((ret = parse_type_name_pair(parser, &sym->name,
					&sym->result_type)) > 0)
		goto err_free_sym;
	if (ret == -1)
		sym->flags.can_null = 1;
	sym->argc = parser->scope->fn->argc;
	sym->args = NULL;
	sym->parse_function = NULL;
	sym->type = SYM_FUNC_ARG;
	if (symbol_register(sym, &parser->scope->sym_groups[SYMG_SYM]))
		goto err_free_sym;
	if (symbol_args_append(parser->scope->fn, sym))
		goto err_free_sym;
	return token_list_elem_end(',', f);
err_free_sym:
	free_safe(sym);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_read_args(struct parser *parser)
{
	if (parser->f->src[parser->f->pos] != '(')
		return 1;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	if (token_parse_list(",)", parser, parser->f, func_def_read_arg))
		goto err_args_cannot_parse;
	if (parser->f->src[parser->f->pos] != ')')
		return 1;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	return 0;
err_args_cannot_parse:
	printf("amc: func_def_read_args: %lld,%lld: "
			"Cannot parse function arguments!\n",
			parser->f->cur_line, parser->f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_read_name(struct parser *parser)
{
	const char *tok_end = " \t\n(:;";
	str token = TOKEN_NEW;
	if (token_read_before(tok_end, &token, parser->f) == NULL)
		goto err_eof;
	file_skip_space(parser->f);
	keyword_end(parser->f);
	if (token.len == 3 && strncmp("rec", token.s, 3) == 0) {
		parser->scope->fn->flags.rec = 1;
		token.len = 0;
		if (token_read_before(tok_end, &token, parser->f) == NULL)
			goto err_eof;
		file_skip_space(parser->f);
		keyword_end(parser->f);
	}
	str_copy(&token, &parser->scope->fn->name);
	if (backend_call(symbol_get_path)(&parser->scope->fn->path,
				&parser->path,
				parser->scope->fn->name.s,
				parser->scope->fn->name.len))
		goto err_get_path_failed;
	return 0;
err_eof:
	printf("amc: func_def_read_name: end of file\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_get_path_failed:
	printf("amc: func_def_read_name: Get symbol path failed!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_read_type(struct parser *parser)
{
	int ret = 0;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	if (parser->f->src[parser->f->pos] != ':')
		goto err_cannot_get_type;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	if ((ret = parse_type(parser, &parser->scope->fn->result_type)) > 0)
		goto err_cannot_get_type;
	if (ret == -1)
		parser->scope->fn->flags.can_null = 1;
	return 0;
err_cannot_get_type:
	printf("amc: func_def_read_type: %lld,%lld: Cannot get type!\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

yz_val *func_ret_get_val(struct symbol *fn, struct expr *expr)
{
	yz_val *result = identifier_handle_expr_val(expr, &fn->result_type);
	if (result == NULL)
		return NULL;
	if (result->type.type == AMC_SYM) {
		if (fn->result_type.type == YZ_PTR)
			if (!comptime_ptr_check_can_ret(result->v, fn))
				goto err_free_result;
		return result;
	}
	return result;
err_free_result:
	free_yz_val(result);
	return NULL;
}

int parse_func_call(struct parser *parser)
{
	yz_val **args = NULL;
	if (parser->sym->argc == 0 && parser->f->src[parser->f->pos] == ']') {
		file_pos_next(parser->f);
		file_skip_space(parser->f);
	} else {
		args = func_call_read_args(parser);
		if (args == NULL)
			return 1;
	}
	if (hook_apply(&parser->sym->hooks->times[HOOK_FUNC_CALL_BEFORE]))
		return 1;
	if (backend_call(func_call)(parser->sym,
				args, parser->sym->argc))
		return 1;
	if (hook_apply(&parser->sym->hooks->times[HOOK_FUNC_CALL_AFTER]))
		return 1;
	return 0;
}

int parse_func_def(struct parser *parser)
{
	struct scope *dest_scope = parser->stat.has_pub
		? parser->scope_pub : parser->scope;
	struct symbol *result = calloc(1, sizeof(*result));
	int ret = 0;
	struct scope fn_scope = {
		.fn = result,
		.indent = parser->scope->indent,
		.parent = parser->scope,
		.status = backend_call(scope_begin)(),
		.status_type = SCOPE_IN_BLOCK,
		.sym_groups = {}
	};
	parser->scope = &fn_scope;
	if (scope_check_is_correct(&fn_scope))
		return 1;
	if (func_def_read_name(parser))
		goto err_free_result;
	if (func_def_check_main(result->name.s, result->name.len))
		return func_def_main(parser);
	if (parser->f->src[parser->f->pos] != ':'
			&& func_def_read_args(parser))
		goto err_free_result;
	if (func_def_read_type(parser))
		goto err_free_result;
	if (func_def_inherit_decorators(&parser->stat.decorators, result))
		goto err_free_result;
	result->flags.in_block = 1;
	result->parse_function = parse_func_call;
	result->type = SYM_FUNC;
	if ((ret = func_def_block_start(parser)) > 0)
		goto err_free_result;
	if (symbol_register(result, &dest_scope->sym_groups[SYMG_FUNC]))
		goto err_free_result;
	if (ret == -1)
		return func_def_end_scope(parser);
	if (backend_call(func_def)(result, parser->stat.has_pub, 0))
		goto err_free_result;
	if (parse_block(parser))
		goto err_free_result;
	return func_def_end_scope(parser);
err_free_result:
	free_symbol(result);
	return 1;
}

int parse_func_ret(struct parser *parser)
{
	struct expr *expr = NULL;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	yz_val *val = {};
	if ((expr = parse_expr(parser, 1)) == NULL)
		return err_print_pos(__func__, "Cannot parse expr!",
				orig_line, orig_column);
	if (expr_apply(parser, expr) > 0)
		goto err_cannot_apply_expr;
	keyword_end(parser->f);
	if ((val = func_ret_get_val(parser->scope->fn, expr)) == NULL)
		goto err_get_val_failed;
	if (backend_call(func_ret)(val, strncmp(parser->scope->fn->name.s,
					"main", 4) == 0))
		goto err_backend_failed;
	free_yz_val(val);
	return 0;
err_cannot_apply_expr:
	free_expr(expr);
	return err_print_pos(__func__, "Cannot apply expr!",
			orig_line, orig_column);
err_get_val_failed:
	return err_print_pos(__func__, "Get value failed!",
			orig_line, orig_column);
err_backend_failed:
	free_yz_val(val);
	return err_print_pos(__func__, "Backend call failed!",
			orig_line, orig_column);
}

int func_call_read(struct parser *parser, struct symbol **fn)
{
	struct symbol *callee = NULL;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	if (func_call_read_callee(parser, &callee))
		return 1;
	if (parser->scope->fn == callee && callee->flags.rec == 0)
		goto err_func_cannot_rec;
	parser->sym = callee;
	if (callee->parse_function(parser))
		return 1;
	if (fn != NULL)
		*fn = callee;
	return 0;
err_func_cannot_rec:
	printf("amc: func_call_read: %lld,%lld: "
			"Function: '%s' cannot be recursively called\n",
			orig_line, orig_column, callee->name.s);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}
