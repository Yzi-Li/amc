#include "include/block.h"
#include "include/expr.h"
#include "include/func.h"
#include "include/indent.h"
#include "include/keywords.h"
#include "../include/backend.h"
#include "../include/file.h"
#include "../include/parser.h"
#include "../include/scope.h"
#include "../include/token.h"
#include "../utils/str/str.h"
#include <stdio.h>

static int block_parse_expr(struct parser *parser);
static int block_parse_func(struct parser *parser);
static int block_parse_keyword(struct parser *parser);
static int block_parse_line(struct parser *parser);

int block_parse_expr(struct parser *parser)
{
	struct expr *expr = NULL;
	if ((expr = parse_expr(parser, 1)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(parser, expr) > 0)
		goto err_cannot_apply_expr;
	free_expr(expr);
	file_pos_next(parser->f);
	return 0;
err_cannot_parse_expr:
	printf("| block_parse_expr: %lld,%lld: Cannot parse expression!\n",
			parser->f->cur_line, parser->f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("| block_parse_expr: %lld,%lld: Cannot apply expression!\n",
			parser->f->cur_line, parser->f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int block_parse_func(struct parser *parser)
{
	if (func_call_read(parser, NULL))
		return 1;
	file_skip_space(parser->f);
	return keyword_end(parser->f);
}

int block_parse_keyword(struct parser *parser)
{
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line,
	    orig_pos = parser->f->pos;
	struct symbol *sym = NULL;
	str token = TOKEN_NEW;
	if (token_next(&token, parser->f))
		return 1;
	if (!keyword_find(&token, &sym)) {
		parser->f->cur_column = orig_column;
		parser->f->cur_line = orig_line;
		parser->f->pos = orig_pos;
		return -1;
	}
	if (!sym->flags.in_block)
		goto err_not_in_block;
	parser->sym = sym;
	return sym->parse_function(parser);
err_not_in_block:
	printf("amc: block_parse_keyword: Symbol unsupport used in block!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int block_parse_line(struct parser *parser)
{
	int indent = 0, ret = 0;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line,
	    orig_pos = parser->f->pos;
	if ((indent = indent_read(parser->f)) != parser->scope->indent) {
		parser->f->cur_column = orig_column;
		parser->f->cur_line = orig_line;
		parser->f->pos = orig_pos;
		return -1;
	}
	file_skip_space(parser->f);
	if (parse_comment(parser->f))
		return 0;
	if (parser->f->src[parser->f->pos] == '\n')
		return file_line_next(parser->f);
	if (parser->f->src[parser->f->pos] == '(')
		return block_parse_expr(parser);
	if (parser->f->src[parser->f->pos] == '[')
		return block_parse_func(parser);
	if ((ret = block_parse_keyword(parser)) == 0)
		return 0;
	if (ret > 0)
		return 1;
	return block_parse_expr(parser);
}

int block_check_start(struct file *f)
{
	str expect = {.len = 2, .s = "=>"};
	return token_try_read(&expect, f);
}

int parse_block(struct parser *parser)
{
	int ret = 0;
	struct scope cur_scope = {
		.fn = parser->scope->fn,
		.indent = parser->scope->indent + 1,
		.parent = parser->scope,
		.status = backend_call(scope_begin)(),
		.status_type = SCOPE_IN_BLOCK,
		.sym_groups = {}
	};
	if (scope_check_is_correct(&cur_scope))
		return 1;
	parser->scope = &cur_scope;
	while ((ret = block_parse_line(parser)) != -1) {
		if (ret > 0)
			return 1;
	}
	parser->scope = cur_scope.parent;
	scope_end(&cur_scope);
	return 0;
}
