#include "keywords.h"
#include "expr.h"
#include "../include/backend.h"
#include "../include/expr.h"
#include "../include/identifier.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../utils/die.h"
#include "../utils/str/str.h"
#include <string.h>

static int let_check_defined(struct file *f, str *name, struct scope *scope);
static int let_check_val_type(yz_val *src, enum YZ_TYPE dest);
static yz_val *let_expr_val_handle(struct expr **e, struct symbol *sym);
static int let_initialize_val(struct file *f, struct symbol *sym,
		struct scope *scope);
static int let_read_def(struct file *f, str *name, str *type);
static struct symbol *let_reg_sym(struct file *f, str *name, int mut,
		struct scope *scope);

int let_check_defined(struct file *f, str *name, struct scope *scope)
{
	char *err_msg;
	struct symbol *sym;
	if (symbol_find_in_group_in_scope(name, &sym, scope, SYMG_SYM))
		goto err_sym_defined;
	return 0;
err_sym_defined:
	err_msg = tok2str(name->s, name->len);
	printf("amc: parse_let: Symbol defined!\n"
			"| Token: \"%s\"\n"
			"| In l:%lld\n",
			err_msg,
			f->cur_line);
	backend_stop(BE_STOP_SIGNAL_ERR);
	free(err_msg);
	return 1;
}

int let_check_val_type(yz_val *src, enum YZ_TYPE dest)
{
	enum YZ_TYPE type = src->type;
	if (src->type == AMC_EXPR) {
		type = *((struct expr*)src->v)->sum_type;
	} else if (src->type == AMC_SYM) {
		type = ((struct symbol*)src->v)->result_type;
	}
	if (type == dest)
		return 0;
	if (YZ_IS_DIGIT(type) && YZ_IS_DIGIT(dest))
		return 0;
	printf("amc: let_check_val_type: Wrong type!\n"
			"| Symbol type: \"%s\"\n"
			"| Value type:  \"%s\"\n",
			yz_get_type_name(dest),
			yz_get_type_name(src->type));
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

yz_val *let_expr_val_handle(struct expr **e, struct symbol *sym)
{
	yz_val *val = NULL;
	if ((*e)->op == NULL && (*e)->valr == NULL) {
		val = (*e)->vall;
		free_safe((*e)->op);
		free_safe((*e)->valr);
		free_safe(*e);
		if (let_check_val_type(val, sym->result_type))
			goto err_get_line;
		return val;
	}
	val = calloc(1, sizeof(*val));
	val->type = AMC_EXPR;
	val->v = *e;
	if (let_check_val_type(val, sym->result_type))
		goto err_get_line;
	return val;
err_get_line:
	printf("|< amc: let_expr_val_handle\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
}

int let_initialize_val(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct expr *expr = NULL;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	yz_val *val = NULL;
	char *name = NULL;
	file_pos_next(f);
	file_skip_space(f);
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(expr, scope) > 0)
		goto err_cannot_apply_expr;
	if ((val = let_expr_val_handle(&expr, sym)) == NULL)
		goto err_cannot_apply_expr;
	name = tok2str(sym->name, sym->name_len); // don't free
	if (sym->flags.mut) {
		if (backend_call(var_set)(name, val))
			return 1;
	} else {
		if (backend_call(var_immut_init)(name, val))
			return 1;
	}
	if (expr != NULL)
		expr_free(expr);
	if (f->src[f->pos] == ']')
		file_pos_next(f);
	if (parse_comment(f))
		return 0;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	return 0;
err_cannot_parse_expr:
	printf("amc: let_initialize_val: Cannot parse expr!\n"
			"| In l:%lld,c:%lld\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("amc: let_initialize_val: Cannot apply expr!\n"
			"| In l:%lld,c:%lld\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int let_read_def(struct file *f, str *name, str *type)
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
	if (token_next(type, f))
		return 2;
	return mut;
err_type_indicator_not_found:
	err_msg = tok2str(name->s, name->len);
	printf("amc: let_read_def: %lld,%lld: Type indicator not found!\n"
			"| Name(Token): \"%s\"\n",
			orig_line, orig_column,
			err_msg),
	backend_stop(BE_STOP_SIGNAL_ERR);
	free(err_msg);
	return 2;
}

struct symbol *let_reg_sym(struct file *f, str *name, int mut,
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
	if (mut) {
		result->flags.mut = 1;
		result->parse_function = parse_mut_var;
	} else {
		result->flags.mut = 0;
		result->parse_function = parse_immut_var;
	}
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
	int mut = 0, ret = 0;
	str name_tok = TOKEN_NEW,
	    type_tok = TOKEN_NEW;
	enum YZ_TYPE type = AMC_ERR_TYPE;
	struct symbol *result = NULL;
	if ((mut = let_read_def(f, &name_tok, &type_tok)) > 1)
		return 1;
	if ((ret = parse_type(&type_tok, &type)))
		goto err_unsupport_type;
	if ((result = let_reg_sym(f, &name_tok, mut, scope)) == NULL)
		return 1;
	result->result_type = type;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	if (parse_comment(f))
		return 0;
	if (f->src[f->pos] != '=')
		goto err_syntax_err;
	return let_initialize_val(f, result, scope);
err_unsupport_type:
	printf("amc: parse_let: Unsupport type!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_syntax_err:
	printf("amc: parse_let: %lld,%lld: Syntax error!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_immut_var(struct file *f, struct symbol *sym, struct scope *scope)
{
	return 1;
}

int parse_mut_var(struct file *f, struct symbol *sym, struct scope *scope)
{
	return 0;
}
