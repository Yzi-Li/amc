#include "include/block.h"
#include "include/expr.h"
#include "include/func.h"
#include "include/indent.h"
#include "include/keywords.h"
#include "../include/backend.h"
#include "../include/file.h"
#include "../include/scope.h"
#include "../include/token.h"
#include "../utils/str/str.h"
#include <stdio.h>

static int block_parse_expr(struct file *f, struct scope *scope);
static int block_parse_func(struct file *f, struct scope *scope);
static int block_parse_keyword(struct file *f, struct scope *scope);
static int block_parse_line(struct file *f, struct scope *scope);

int block_parse_expr(struct file *f, struct scope *scope)
{
	struct expr *expr = NULL;
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(expr, scope) > 0)
		goto err_cannot_apply_expr;
	expr_free(expr);
	file_pos_next(f);
	return 0;
err_cannot_parse_expr:
	printf("| block_parse_expr: %lld,%lld: Cannot parse expression!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("| block_parse_expr: %lld,%lld: Cannot apply expression!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int block_parse_func(struct file *f, struct scope *scope)
{
	if (func_call_read(f, NULL, scope))
		return 1;
	file_skip_space(f);
	return keyword_end(f);
}

int block_parse_keyword(struct file *f, struct scope *scope)
{
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line,
	    orig_pos = f->pos;
	struct symbol *sym = NULL;
	str token = TOKEN_NEW;
	if (token_next(&token, f))
		return 1;
	if (!keyword_find(&token, &sym)) {
		f->cur_column = orig_column;
		f->cur_line = orig_line;
		f->pos = orig_pos;
		return -1;
	}
	if (!sym->flags.in_block)
		goto err_not_in_block;
	return sym->parse_function(f, sym, scope);
err_not_in_block:
	printf("amc: block_parse_keyword: Symbol unsupport used in block!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int block_parse_line(struct file *f, struct scope *scope)
{
	int indent = 0, ret = 0;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line,
	    orig_pos = f->pos;
	if ((indent = indent_read(f)) != scope->indent) {
		f->cur_column = orig_column;
		f->cur_line = orig_line;
		f->pos = orig_pos;
		return -1;
	}
	file_skip_space(f);
	if (parse_comment(f))
		return 0;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	if (f->src[f->pos] == '(')
		return block_parse_expr(f, scope);
	if (f->src[f->pos] == '[')
		return block_parse_func(f, scope);
	if ((ret = block_parse_keyword(f, scope)) == 0)
		return 0;
	if (ret > 0)
		return 1;
	return block_parse_expr(f, scope);
}

int parse_block(struct file *f, struct scope *scope)
{
	int ret = 0;
	struct scope cur_scope = {
		.fn = scope->fn,
		.indent = scope->indent + 1,
		.parent = scope,
		.status = backend_call(scope_begin)(),
		.status_type = SCOPE_IN_BLOCK,
		.sym_groups = {}
	};
	if (scope_check_is_correct(&cur_scope))
		return 1;
	while ((ret = block_parse_line(f, &cur_scope)) != -1) {
		if (ret > 0)
			return 1;
	}
	scope_end(&cur_scope);
	return 0;
}
