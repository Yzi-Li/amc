#include "include/array.h"
#include "include/expr.h"
#include "include/identifier.h"
#include "include/keywords.h"
#include "include/null.h"
#include "include/type.h"
#include "../include/backend.h"
#include "../include/expr.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../utils/str/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int identifier_assign_backend_call(struct symbol *sym, yz_val *val,
		enum OP_ID mode, struct scope *scope);
static int let_check_defined(struct file *f, str *name, struct scope *scope);
static int let_check_val_type(yz_val *src, yz_val *dest);
static int let_handle_val_type(yz_val *src, yz_val *dest);
static int let_init_constructor(struct file *f, struct symbol *sym,
		struct scope *scope);
static int let_init_null(struct file *f, struct symbol *sym,
		struct scope *scope);
static int let_init_val(struct file *f, struct symbol *sym,
		struct scope *scope);
static int let_read_def(struct file *f, str *name);
static int let_read_def_type(struct file *f, yz_val *type);
static struct symbol *let_reg_sym(struct file *f, str *name,
		int can_null, int mut,
		struct scope *scope);

int identifier_assign_backend_call(struct symbol *sym, yz_val *val,
		enum OP_ID mode, struct scope *scope)
{
	char *name = str2chr(sym->name, sym->name_len);
	if (sym->flags.mut) {
		if (backend_call(var_set)(name, val, mode, scope->status))
			goto err_free_name;
	} else {
		if (sym->flags.is_init)
			goto err_sym_is_immut;
		if (mode != OP_ASSIGN)
			goto err_unsupport_op;
		if (backend_call(var_immut_init)(name, val, scope->status))
			goto err_free_name;
	}
	sym->flags.is_init = 1;
	free(name);
	return 0;
err_free_name:
	free(name);
	return 1;
err_sym_is_immut:
	printf("amc: identifier_assign_backend_call: "
			"Symbol: \"%s\" is immutable!\n", name);
	goto err_free_name;
err_unsupport_op:
	printf("amc: identifier_assign_backend_call: "
			"Unsupport operator for immutable identifier!\n");
	goto err_free_name;
}

int let_check_defined(struct file *f, str *name, struct scope *scope)
{
	char *err_msg;
	struct symbol *sym;
	if (symbol_find_in_group_in_scope(name, &sym, scope, SYMG_SYM))
		goto err_sym_defined;
	return 0;
err_sym_defined:
	err_msg = str2chr(name->s, name->len);
	printf("amc: let_check_defined: %lld,%lld: Symbol defined!\n"
			"| Token: \"%s\"\n",
			f->cur_line, f->cur_column,
			err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	free(err_msg);
	return 1;
}

int let_check_val_type(yz_val *src, yz_val *dest)
{
	yz_val *val = NULL;
	if ((val = yz_type_max(src, dest)) == NULL)
		goto err_wrong_type;
	return 0;
err_wrong_type:
	printf("amc: let_check_val_type: Wrong type!\n"
			"| Symbol type: \"%s\"\n"
			"| Value type:  \"%s\"\n",
			yz_get_type_name(dest),
			yz_get_type_name(src));
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int let_handle_val_type(yz_val *src, yz_val *dest)
{
	if (let_check_val_type(src, dest))
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
		return array_structure(f, sym, scope);
		break;
	default:
		return 1;
	}
	return 0;
}

int let_init_null(struct file *f, struct symbol *sym, struct scope *scope)
{
	char *name = NULL;
	yz_val val = {.type = YZ_NULL, .v = NULL};
	file_pos_nnext(strlen(CHR_NULL), f);
	file_skip_space(f);
	if (f->src[f->pos] != '\n' && f->src[f->pos] != ';')
		goto err_null_must_exist_separately;
	if (!sym->flags.can_null)
		goto err_identifier_cannot_be_null;
	name = str2chr(sym->name, sym->name_len);
	if (sym->flags.mut) {
		if (backend_call(var_set)(name, &val, OP_ASSIGN,
					scope->status))
			goto err_free_name;
	} else {
		if (backend_call(var_immut_init)(name, &val, scope->status))
			goto err_free_name;
	}
	free(name);
	return keyword_end(f);
err_null_must_exist_separately:
	printf("amc: let_init_null: %lld,%lld: null must exist separately!\n",
			f->cur_line, f->cur_column);
	return 1;
err_identifier_cannot_be_null:
	printf("amc: let_init_null: %lld,%lld: "
			"Identifier cannot be null!\n",
			f->cur_line, f->cur_column);
	return 1;
err_free_name:
	free(name);
	printf("amc: let_init_null: Backend call failed!\n");
	return 1;
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

int let_read_def(struct file *f, str *name)
{
	char *err_msg;
	int mut = 0;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	if (token_next(name, f))
		return 2;
	if (name->len == 3 && strncmp(name->s, "mut", 3) == 0) {
		name->len = 0;
		if (token_next(name, f))
			return 2;
		mut = 1;
	}
	if (name->s[name->len - 1] != ':')
		goto err_type_indicator_not_found;
	name->len -= 1;
	return mut;
err_type_indicator_not_found:
	err_msg = str2chr(name->s, name->len);
	printf("amc: let_read_def: %lld,%lld: Type indicator not found!\n"
			"| Name(Token): \"%s\"\n",
			orig_line, orig_column,
			err_msg),
	backend_stop(BE_STOP_SIGNAL_ERR);
	free(err_msg);
	return 2;
}

int let_read_def_type(struct file *f, yz_val *type)
{
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	int ret = 0;
	if ((ret = parse_type(f, type)) > 0)
		goto err_unsupport_type;
	return ret;
err_unsupport_type:
	printf("amc: parse_let: %lld,%lld: Unsupport type!\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

struct symbol *let_reg_sym(struct file *f, str *name, int can_null, int mut,
		struct scope *scope)
{
	struct symbol *result = NULL;
	if (let_check_defined(f, name, scope))
		return NULL;
	result = calloc(1, sizeof(*result));
	result->argc = 1;
	result->args = NULL;
	result->name = name->s;
	result->name_len = name->len;
	result->flags.can_null = can_null;
	result->flags.mut = mut;
	result->parse_function = NULL;
	if (symbol_register(result, &scope->sym_groups[SYMG_SYM]))
		goto err_cannot_register_sym;
	return result;
err_cannot_register_sym:
	printf("amc: let_reg_sym: %lld,%lld: Cannot register symbol!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	free_safe(result);
	return NULL;
}

int parse_let(struct file *f, struct symbol *sym, struct scope *scope)
{
	int can_null = 0, mut = 0;
	str name_tok = TOKEN_NEW;
	yz_val type = {};
	struct symbol *result = NULL;
	if ((mut = let_read_def(f, &name_tok)) > 1)
		return 1;
	if ((can_null = let_read_def_type(f, &type)) > 0)
		return 1;
	if ((result = let_reg_sym(f, &name_tok, can_null, mut, scope)) == NULL)
		return 1;
	result->result_type = type;
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
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int identifier_assign_val(struct file *f, struct symbol *sym, enum OP_ID mode,
		struct scope *scope)
{
	struct expr *expr = NULL;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	yz_val *val = NULL;
	sym->flags.comptime_flag.checked_null = 0;
	if (strncmp(CHR_NULL, &f->src[f->pos], strlen(CHR_NULL)) == 0)
		return let_init_null(f, sym, scope);
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(expr, scope) > 0)
		goto err_cannot_apply_expr;
	if ((val = identifier_expr_val_handle(&expr, &sym->result_type))
			== NULL)
		goto err_cannot_apply_expr;
	if (identifier_assign_backend_call(sym, val, mode, scope))
		goto err_backend_failed;
	if (expr != NULL)
		expr_free(expr);
	if (f->src[f->pos] == ']')
		file_pos_next(f);
	return 0;
err_cannot_parse_expr:
	printf("| identifier_assign_val: %lld,%lld: Cannot parse expr!\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("| identifier_assign_val: %lld,%lld: Cannot apply expr!\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_backend_failed:
	printf("amc: identifier_assign_val: %lld,%lld: Backend call failed!\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

yz_val *identifier_expr_val_handle(struct expr **e, yz_val *type)
{
	yz_val *val = NULL;
	if ((*e)->op == NULL && (*e)->valr == NULL) {
		val = (*e)->vall;
		if (let_handle_val_type(val, type))
			goto err_get_type;
		return val;
	}
	val = calloc(1, sizeof(*val));
	val->type = AMC_EXPR;
	val->v = *e;
	if (let_handle_val_type(val, type))
		goto err_get_type;
	return val;
err_get_type:
	printf("|< amc: let_expr_val_handle\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
}

int identifier_read(struct file *f, str *token, yz_val *val,
		struct scope *scope)
{
	char *err_msg;
	struct symbol *sym = NULL;
	if (!symbol_find_in_group_in_scope(token, &sym, scope, SYMG_SYM))
		goto err_identifier_not_found;
	val->type = AMC_SYM;
	val->v = sym;
	if (f->src[f->pos] != '[')
		return 0;
	if (sym->result_type.type == YZ_ARRAY)
		return array_get_elem(f, val, scope);
	return 0;
err_identifier_not_found:
	err_msg = str2chr(token->s, token->len);
	printf("amc: identifier_read: %lld,%lld: Identifier not found!\n"
			"| Token: \"%s\"\n",
			f->cur_line, f->cur_column,
			err_msg);
	free(err_msg);
	return 2;
}
