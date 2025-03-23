#include "../include/backend.h"
#include "../include/file.h"
#include "../include/identifier.h"
#include "../include/parser.h"
#include "../include/scope.h"
#include "../include/token.h"
#include "../include/type.h"
#include "../utils/converter.h"
#include "../utils/die.h"
#include "../utils/utils.h"
#include "expr.h"
#include "block.h"
#include "keywords.h"
#include <limits.h>
#include <stdio.h>

struct func_call_handle {
	struct file *f;
	struct symbol *fn;
	int index;
	struct scope *scope;
	yz_val **vals;
};

static int func_call_main(struct file *f, struct symbol *sym,
		struct scope *scope);
static int func_call_read_arg(const char *se, struct file *f, void *data);
static yz_val **func_call_read_args(struct file *f, struct symbol *fn,
		struct scope *scope);
static int func_def_block_start(struct file *f);
static int func_def_check_main(const char *name, int len);
static int func_def_main(struct file *f, struct scope *scope);
static int func_def_read_arg(const char *se, struct file *f, void *data);
static int func_def_read_args(struct file *f, struct symbol *fn,
		struct scope *scope);
static int func_def_read_block(struct file *f, struct scope *scope);
static int func_def_read_name(struct file *f, struct symbol *fn);
static int func_def_read_type(struct file *f, struct symbol *fn);
static int func_def_reg_arg(str *name, str *type_tok, struct scope *scope);

int func_call_main(struct file *f, struct symbol *sym, struct scope *scope)
{
	printf("amc: You cannot call the main function!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_call_read_arg(const char *se, struct file *f, void *data)
{
	struct expr *expr = NULL;
	struct func_call_handle *handle = data;
	yz_val *result = NULL;
	int ret = 0;
	enum YZ_TYPE type = AMC_ERR_TYPE;
	if (handle->index > handle->fn->argc)
		goto err_too_many_args;
	if ((expr = parse_expr(f, 1, handle->scope)) == NULL)
		goto err_cannot_parse_arg;
	if ((ret = expr_apply(expr)) > 0)
		goto err_cannot_parse_arg;
	if (ret == -1) {
		result = expr->vall;
		free_safe(expr->op);
		free_safe(expr->valr);
		free_safe(expr);
		if (result->type == AMC_EXPR) {
			type = *((struct expr*)result->v)->sum_type;
		} else if (result->type == AMC_SYM) {
			type = ((struct symbol*)result->v)->result_type;
		} else {
			type = result->type;
		}
	} else {
		result = malloc(sizeof(*result));
		result->type = AMC_EXPR;
		result->v = expr;
		type = *((struct expr*)result->v)->sum_type;
	}
	if (type != handle->fn->args[handle->index]
			&& result->type != AMC_EXPR) {
		if (!YZ_IS_DIGIT(handle->fn->args[handle->index])
				|| !REGION_INT(type, YZ_I8, YZ_U64))
			goto err_wrong_arg_type;
		result->type = handle->fn->args[handle->index];
	}
	handle->vals[handle->index] = result;
	handle->index += 1;
	return 0;
err_too_many_args:
	printf("amc: func_call_read_arg: Too many parameters.\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_parse_arg:
	printf("amc: func_call_read_arg: Cannot parse argument.\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_wrong_arg_type:
	printf("amc: func_call_read_arg: Wrong argument type: \"%s\"\n"
			"| In l:%lld,c:%lld\n",
			yz_get_type_name(type),
			f->cur_line, f->cur_column);
	return 1;
}

yz_val **func_call_read_args(struct file *f, struct symbol *fn,
		struct scope *scope)
{
	yz_val **result = calloc(fn->argc, sizeof(*result));
	struct func_call_handle *handle = malloc(sizeof(*handle));
	handle->f = f;
	handle->fn = fn;
	handle->index = 0;
	handle->scope = scope;
	handle->vals = result;
	if (token_parse_list(",]", handle, f, func_call_read_arg))
		goto err_free_result;
	free_safe(handle);
	return result;
err_free_result:
	free_safe(handle);
	for (int i = 0; i < fn->argc; i++)
		free_safe(result[i]);
	free_safe(result);
	return NULL;
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
	err_msg = err_msg_get(token.s, token.len);
	printf("amc: func_def_block_start:\n"
			"| Function define start character not found\n"
			"| Token: \"%s\"\n"
			"|         ^\n"
			"| In l:%lld,c:%lld\n",
			err_msg,
			f->cur_line, f->cur_column);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_check_main(const char *name, int len)
{
	const char *template = "main";
	const int template_len = 4;

	if (len != template_len)
		return 0;
	if (strncmp(name, template, len) == 0)
		return 1;

	return 0;
}

int func_def_main(struct file *f, struct scope *scope)
{
	scope->fn->result_type = YZ_I8;
	parser_global_conf.has_main = 1;
	while (f->src[f->pos] != '=') {
		file_pos_next(f);
	}
	if (backend_call(func_def)("_start", 7, YZ_VOID))
		goto err_free_fn;
	scope->fn->parse_function = func_call_main;
	if (symbol_register(scope->fn, &scope->sym_groups[SYMG_FUNC]))
		goto err_free_fn;
	return func_def_read_block(f, scope);
err_free_fn:
	free_safe(scope->fn);
	return 1;
}

int func_def_read_arg(const char *se, struct file *f, void *data)
{
	char *end = NULL;
	str token = TOKEN_NEW,
	    type_tok = TOKEN_NEW;
	if ((end = token_read_before(se, &token, f)) == NULL)
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	if (token_type_get(&token, &type_tok))
		return 1;
	if (func_def_reg_arg(&token, &type_tok, (struct scope*)data))
		return 1;
	if (*end == se[1])
		return -1;
	return 0;
}

int func_def_read_args(struct file *f, struct symbol *fn, struct scope *scope)
{
	if (f->src[f->pos] == '(')
		file_pos_next(f);
	if (token_parse_list(",)", scope, f, func_def_read_arg))
		goto err_args_cannot_parse;
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
	return parse_block(0, f, scope);
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

int func_def_read_type(struct file *f, struct symbol *fn)
{
	enum YZ_TYPE type = AMC_ERR_TYPE;
	str token = TOKEN_NEW;
	int ret = 0;
	if (token_next(&token, f))
		return 1;
	if (token.s[0] != ':')
		return 1;
	if (token.len == 1) {
		token.len = 0;
		if (token_next(&token, f))
			return 1;
	} else {
		token.s = &token.s[1];
		token.len -= 1;
	}
	ret = parse_type(&token, &type);
	if (ret)
		goto err_unsupport_type;
	if (type == AMC_ERR_TYPE)
		goto err_cannot_get_type;
	fn->result_type = type;
	fn->parse_function = parse_func_call;
	return 0;
err_unsupport_type:
	printf("amc: func_def_read_type: Unsupport type!");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_get_type:
	printf("amc: func_def_read_type: Cannot get function return type!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int func_def_reg_arg(str *name, str *type_tok, struct scope *scope)
{
	struct symbol *sym = NULL;
	enum YZ_TYPE type = AMC_ERR_TYPE;
	int ret = parse_type(type_tok, &type);
	if (type == AMC_ERR_TYPE)
		return 1;
	if (ret)
		goto err_unsupport_type;
	sym = calloc(1, sizeof(*sym));
	sym->name = name->s;
	sym->name_len = name->len;
	sym->parse_function = parse_immut_var;
	if (symbol_register(sym, &scope->sym_groups[SYMG_SYM]))
		goto err_free_sym;
	if (symbol_args_append(scope->fn, type))
		goto err_free_sym;
	return 0;
err_free_sym:
	free_safe(sym);
	return 1;
err_unsupport_type:
	printf("amc: func_def_reg_arg: Unsupport type!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_func_call(struct file *f, struct symbol *sym, struct scope *scope)
{
	yz_val **v = NULL;
	char *name = NULL;
	int ret = 0;
	v = func_call_read_args(f, sym, scope);
	if (v == NULL)
		return 1;
	if (sym->name[sym->name_len - 1] != '\0') {
		name = malloc(sym->name_len + 1);
		memcpy(name, sym->name, sym->name_len);
		name[sym->name_len] = '\0';
		ret = backend_call(func_call)(name, v, sym->argc);
		if (ret)
			return 1;
	} else {
		ret = backend_call(func_call)(sym->name, v, sym->argc);
		if (ret)
			return 1;
	}
	return ret;
}

int parse_func_def(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct symbol *result = calloc(1, sizeof(*result));
	struct scope fn_scope = {
		.fn = result,
		.parent = scope,
		.sym_groups = {}
	};
	if (func_def_read_name(f, result))
		goto err_free_result;
	if (func_def_check_main(result->name, result->name_len))
		return func_def_main(f, &fn_scope);
	if (f->src[f->pos] != ':' && func_def_read_args(f, result, &fn_scope))
		goto err_free_result;
	if (func_def_read_type(f, result))
		goto err_free_result;
	if (backend_call(func_def)(result->name,
			result->name_len,
			result->result_type))
		goto err_free_result;
	result->flags.in_block = 1;
	if (symbol_register(result, &scope->sym_groups[SYMG_FUNC]))
		goto err_free_result;
	if (func_def_read_block(f, &fn_scope))
		goto err_free_result;
	scope_free(&fn_scope);
	return 0;
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
	if ((ret = expr_apply(expr)) > 0)
		goto err_free_expr;
	is_main = (scope->fn->name_len == 4
			&& strncmp(scope->fn->name, "main", 4) == 0);
	val.type = expr->vall->type;
	val.l = expr->vall->l;
	if (*expr->sum_type != scope->fn->result_type
			&& REGION_INT(*expr->sum_type, YZ_I8, YZ_U64)) {
		val.type = scope->fn->result_type;
	}
	if (ret == 0) {
		val.type = AMC_EXPR;
		val.v = expr;
	}
	if (backend_call(func_ret)(&val, is_main))
		return 1;
	expr_free(expr);
	return 0;
err_free_expr:
	expr_free(expr);
	return 1;
}
