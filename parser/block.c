#include "block.h"
#include "../include/backend.h"
#include "../include/file.h"
#include "../include/token.h"
#include "../utils/str/str.h"
#include "expr.h"
#include "keywords.h"
#include <stdio.h>

static int block_get_indent(struct file *f);
static int block_line_expr(str *tok, int indent, struct file *f,
		struct scope *scope);
static int block_line_keyword(str *tok, int indent, struct file *f,
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

int block_line_expr(str *tok, int indent, struct file *f, struct scope *scope)
{
	struct expr *expr = NULL;
	char *tok_start = tok->s;
	int top = tok->s[0] == '(';
	if (top) {
		tok->s = tok_start;
	} else {
		tok->s = &tok_start[1];
		tok->len -= 2;
	}
	//expr = parse_expr_token(f, 0);
	expr_apply(expr);
	return 0;
}

int block_line_keyword(str *tok, int indent, struct file *f,
		struct scope *scope)
{
	struct symbol *sym = NULL;
	if (token_next(tok, f))
		return 1;
	if (tok->s[0] == '(')
		return -1;
	if (!keyword_find(tok, &sym))
		return -1;
	if (!sym->flags.in_block)
		goto err_not_in_block;
	return sym->parse_function(f, sym, scope);
err_not_in_block:
	printf("amc: block_line_keyword: Symbol unsupport used in block!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int block_parse_line(int indent, struct file *f, struct scope *scope)
{
	int cur_indent = block_get_indent(f), ret = 0;
	str tok = TOKEN_NEW;
	if (cur_indent <= indent)
		return -1;
	file_skip_space(f);
	if (parse_comment(f))
		return 0;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	if ((ret = block_line_keyword(&tok, cur_indent, f, scope)) == 0)
		return 0;
	if (ret > 0)
		return 1;
	if (parse_comment(f))
		return 0;
	//if (block_line_expr(&tok, cur_indent, f, fn) == 0)
	//	return 0;
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
