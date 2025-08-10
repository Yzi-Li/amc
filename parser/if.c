/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/block.h"
#include "include/expr.h"
#include "include/keywords.h"
#include "../include/backend.h"
#include "../include/parser.h"
#include <stdio.h>

static int if_block_parse(struct parser *parser);
static int if_condition_parse(struct parser *parser);

int if_block_parse(struct parser *parser)
{
	if (parse_block(parser))
		goto err_parse_block_failed;
	return 0;
err_parse_block_failed:
	printf("amc: if_block_parse: %lld,%lld: Parse block failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int if_condition_parse(struct parser *parser)
{
	struct expr *expr = NULL;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	if ((expr = parse_expr(parser, 1)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(parser, expr) > 0)
		goto err_cannot_apply_expr;
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

int parse_if(struct parser *parser)
{
	if (if_condition_parse(parser))
		return 1;
	if (backend_call(cond_if_begin)(parser->scope->status))
		goto err_backend_failed;
	if (if_block_parse(parser))
		return 1;
	if (backend_call(cond_if)(parser->scope->status))
		goto err_backend_failed;
	parser->scope->status_type = SCOPE_AFTER_IF;
	return 0;
err_backend_failed:
	printf("amc: parse_if: %lld,%lld: Backend call failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int parse_elif(struct parser *parser)
{
	if (parser->scope->status_type != SCOPE_AFTER_IF)
		goto err_if_block_not_found;
	if (if_condition_parse(parser))
		return 1;
	if (if_block_parse(parser))
		return 1;
	if (backend_call(cond_elif)(parser->scope->status))
		goto err_backend_failed;
	parser->scope->status_type = SCOPE_AFTER_IF;
	return 0;
err_if_block_not_found:
	printf("amc: parse_elif: %lld,%lld: 'if' block not found!\n"
			"| scope->status_type = '%d'\n",
			parser->f->cur_line, parser->f->cur_column,
			parser->scope->status_type);
	return 1;
err_backend_failed:
	printf("amc: parse_elif: %lld,%lld: Backend call failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int parse_else(struct parser *parser)
{
	if (parser->scope->status_type != SCOPE_AFTER_IF)
		goto err_if_block_not_found;
	if (parser->f->src[parser->f->pos] != '='
			|| parser->f->src[parser->f->pos + 1] != '>')
		goto err_block_start_not_found;
	file_line_next(parser->f);
	if (parse_block(parser))
		goto err_parse_block_failed;
	if (backend_call(cond_else)(parser->scope->status))
		goto err_backend_failed;
	parser->scope->status_type = SCOPE_IN_BLOCK;
	return 0;
err_if_block_not_found:
	printf("amc: parse_else: %lld,%lld: 'if' block not found!\n"
			"| scope->status_type = '%d'\n",
			parser->f->cur_line, parser->f->cur_column,
			parser->scope->status_type);
	return 1;
err_block_start_not_found:
	printf("amc: parse_else: %lld,%lld: block start symbol not found!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
err_parse_block_failed:
	printf("amc: parse_else: %lld,%lld: Parse block failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
err_backend_failed:
	printf("amc: parse_else: %lld,%lld: Backend call failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}
