#include "block.h"
#include "../include/backend.h"
#include "../include/file.h"
#include "../include/token.h"
#include "../utils/str/str.h"
#include "expr.h"
#include "keywords.h"
#include <stdio.h>

static int block_get_indent(struct file *f);
static int block_parse_expr(int indent, struct file *f, struct scope *scope);
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
	str token = TOKEN_NEW;
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_cannot_parse_expr;
	return 0;
err_cannot_parse_expr:
	printf("amc: block_parse_expr: Cannot parse expression!\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int block_parse_keyword(int indent, struct file *f, struct scope *scope)
{
	struct symbol *sym = NULL;
	str token = TOKEN_NEW;
	if (token_next(&token, f))
		return 1;
	if (!keyword_find(&token, &sym))
		return -1;
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
	int cur_indent = block_get_indent(f), ret = 0;
	if (cur_indent <= indent)
		return -1;
	file_skip_space(f);
	if (parse_comment(f))
		return 0;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	if (f->src[f->pos] == '(')
		return block_parse_expr(cur_indent, f, scope);
	if ((ret = block_parse_keyword(cur_indent, f, scope)) == 0)
		return 0;
	if (ret > 0)
		return 1;
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
