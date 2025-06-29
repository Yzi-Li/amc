#include "include/block.h"
#include "include/expr.h"
#include "include/keywords.h"
#include "../include/backend.h"
#include <stdio.h>

static int if_block_parse(struct file *f, struct symbol *sym,
		struct scope *scope);
static int if_condition_parse(struct file *f, struct symbol *sym,
		struct scope *scope);

int if_block_parse(struct file *f, struct symbol *sym, struct scope *scope)
{
	if (parse_block(f, scope))
		goto err_parse_block_failed;
	return 0;
err_parse_block_failed:
	printf("amc: if_block_parse: %lld,%lld: Parse block failed!\n",
			f->cur_line, f->cur_column);
	return 1;
}

int if_condition_parse(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct expr *expr = NULL;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(expr, scope) > 0)
		goto err_cannot_apply_expr;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	return 0;
err_cannot_parse_expr:
	printf("|< amc: if_condition_parse: %lld,%lld: "
			"Cannot parse expression!\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("|< amc: if_condition_parse: %lld,%lld: "
			"Cannot apply expression!\n",
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_if(struct file *f, struct symbol *sym, struct scope *scope)
{
	if (if_condition_parse(f, sym, scope))
		return 1;
	if (backend_call(cond_if_begin)(scope->status))
		goto err_backend_failed;
	if (if_block_parse(f, sym, scope))
		return 1;
	if (backend_call(cond_if)(scope->status))
		goto err_backend_failed;
	scope->status_type = SCOPE_AFTER_IF;
	return 0;
err_backend_failed:
	printf("amc: parse_if: %lld,%lld: Backend call failed!\n",
			f->cur_line, f->cur_column);
	return 1;
}

int parse_elif(struct file *f, struct symbol *sym, struct scope *scope)
{
	if (scope->status_type != SCOPE_AFTER_IF)
		goto err_if_block_not_found;
	if (if_condition_parse(f, sym, scope))
		return 1;
	if (if_block_parse(f, sym, scope))
		return 1;
	if (backend_call(cond_elif)(scope->status))
		goto err_backend_failed;
	scope->status_type = SCOPE_AFTER_IF;
	return 0;
err_if_block_not_found:
	printf("amc: parse_elif: %lld,%lld: 'if' block not found!\n"
			"| scope->status_type = '%d'\n",
			f->cur_line, f->cur_column,
			scope->status_type);
	return 1;
err_backend_failed:
	printf("amc: parse_elif: %lld,%lld: Backend call failed!\n",
			f->cur_line, f->cur_column);
	return 1;
}

int parse_else(struct file *f, struct symbol *sym, struct scope *scope)
{
	if (scope->status_type != SCOPE_AFTER_IF)
		goto err_if_block_not_found;
	if (f->src[f->pos] != '=' || f->src[f->pos + 1] != '>')
		goto err_block_start_not_found;
	file_line_next(f);
	if (parse_block(f, scope))
		goto err_parse_block_failed;
	if (backend_call(cond_else)(scope->status))
		goto err_backend_failed;
	scope->status_type = SCOPE_IN_BLOCK;
	return 0;
err_if_block_not_found:
	printf("amc: parse_else: %lld,%lld: 'if' block not found!\n"
			"| scope->status_type = '%d'\n",
			f->cur_line, f->cur_column,
			scope->status_type);
	return 1;
err_block_start_not_found:
	printf("amc: parse_else: %lld,%lld: block start symbol not found!\n",
			f->cur_line, f->cur_column);
	return 1;
err_parse_block_failed:
	printf("amc: parse_else: %lld,%lld: Parse block failed!\n",
			f->cur_line, f->cur_column);
	return 1;
err_backend_failed:
	printf("amc: parse_else: %lld,%lld: Backend call failed!\n",
			f->cur_line, f->cur_column);
	return 1;
}
