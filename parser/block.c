#include "block.h"
#include "../include/backend.h"
#include "../include/file.h"
#include "../include/token.h"
#include "../utils/die.h"
#include "../utils/str/str.h"
#include "expr.h"
#include "keywords.h"
#include <stdio.h>

static int block_get_indent(struct file *f);
static int block_parse_expr(int indent, struct file *f, struct scope *scope);
static int block_parse_func(int indent, struct file *f, struct scope *scope);
static int block_parse_keyword(int indent, struct file *f,
		struct scope *scope);
static int block_parse_line(int indent, struct file *f, struct scope *scope);

int block_get_indent(struct file *f)
{
	int i = 0;
	while (f->src[f->pos] == '\t') {
		file_pos_next(f);
		i++;
	}
	return i;
}

int block_parse_expr(int indent, struct file *f, struct scope *scope)
{
	struct expr *expr = NULL;
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(expr) > 0)
		goto err_cannot_apply_expr;
	expr_free(expr);
	file_pos_next(f);
	return 0;
err_cannot_parse_expr:
	printf("amc: block_parse_expr: Cannot parse expression!\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("amc: block_parse_expr: Cannot apply expression!\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int block_parse_func(int indent, struct file *f, struct scope *scope)
{
	char *err_msg;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	struct symbol *sym = NULL;
	str token = TOKEN_NEW;
	file_pos_next(f);
	file_skip_space(f);
	if (token_next(&token, f))
		return 1;
	if (!symbol_find_in_group_in_scope(&token, &sym, scope, SYMG_FUNC))
		goto err_func_not_found;
	if (sym->parse_function(f, sym, scope))
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	parse_comment(f);
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	return 0;
err_func_not_found:
	err_msg = tok2str(token.s, token.len);
	printf("amc: block_parse_func: Function not found!\n"
			"| Token: \"%s\"\n"
			"| In l:%lld,c:%lld\n",
			err_msg,
			orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	free(err_msg);
	return 1;
}

int block_parse_keyword(int indent, struct file *f, struct scope *scope)
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

int block_parse_line(int indent, struct file *f, struct scope *scope)
{
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line,
	    orig_pos = f->pos;
	int cur_indent = 0, ret = 0;
	if ((cur_indent = block_get_indent(f)) <= indent) {
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
		return block_parse_expr(cur_indent, f, scope);
	if (f->src[f->pos] == '[')
		return block_parse_func(cur_indent, f, scope);
	if ((ret = block_parse_keyword(cur_indent, f, scope)) == 0)
		return 0;
	if (ret > 0)
		return 1;
	if (block_parse_expr(cur_indent, f, scope))
		goto err_not_expr_or_symbol_call;
	return 0;
err_not_expr_or_symbol_call:
	printf("amc: block_parse_line: "
			"Line is not expression or symbol call!\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_block(int indent, struct file *f, struct scope *scope)
{
	int ret = 0;
	while ((ret = block_parse_line(indent, f, scope)) != -1) {
		if (ret > 0)
			return 1;
	}
	return 0;
}
