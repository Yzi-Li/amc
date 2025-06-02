#include "include/expr.h"
#include "include/block.h"
#include "include/keywords.h"
#include "../include/backend.h"
#include <stdio.h>

static int loop_body_parse(struct file *f, struct scope *scope);
static int loop_condition_parse(struct file *f, struct scope *scope);

int loop_body_parse(struct file *f, struct scope *scope)
{
	if (parse_block(f, scope))
		goto err_parse_block_failed;
	return 0;
err_parse_block_failed:
	printf("amc: loop_body_parse: %lld,%lld: Parse block failed!\n",
			f->cur_line, f->cur_column);
	return 1;
}

int loop_condition_parse(struct file *f, struct scope *scope)
{
	struct expr *expr = NULL;
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_parse_expr_failed;
	if (expr_apply(expr, scope) > 0)
		goto err_apply_expr_failed;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	return 0;
err_parse_expr_failed:
	printf("amc: loop_condition_parse: %lld,%lld: "
			"Parse expression failed!\n",
			f->cur_line, f->cur_column);
	return 1;
err_apply_expr_failed:
	printf("amc: loop_condition_parse: %lld,%lld: "
			"Apply expression failed!\n",
			f->cur_line, f->cur_column);
	return 1;
}

int parse_while(struct file *f, struct symbol *sym, struct scope *scope)
{
	if (backend_call(while_begin)(scope->status))
		goto err_backend_failed;
	if (loop_condition_parse(f, scope))
		return 1;
	if (loop_body_parse(f, scope))
		return 1;
	if (backend_call(while_end)(scope->status))
		goto err_backend_failed;
	return 0;
err_backend_failed:
	printf("amc: parse_while: %lld,%lld: Backend call failed!\n",
			f->cur_line, f->cur_column);
	return 1;
}
