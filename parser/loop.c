/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/expr.h"
#include "include/block.h"
#include "include/keywords.h"
#include "../include/backend.h"
#include "../include/parser.h"
#include <stdio.h>

static int loop_body_parse(struct parser *parser);
static int loop_condition_parse(struct parser *parser);

int loop_body_parse(struct parser *parser)
{
	if (parse_block(parser))
		goto err_parse_block_failed;
	return 0;
err_parse_block_failed:
	printf("amc: loop_body_parse: %lld,%lld: Parse block failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int loop_condition_parse(struct parser *parser)
{
	struct expr *expr = NULL;
	if ((expr = parse_expr(parser, 1)) == NULL)
		goto err_parse_expr_failed;
	if (expr_apply(parser, expr) > 0)
		goto err_apply_expr_failed;
	return 0;
err_parse_expr_failed:
	printf("amc: loop_condition_parse: %lld,%lld: "
			"Parse expression failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
err_apply_expr_failed:
	printf("amc: loop_condition_parse: %lld,%lld: "
			"Apply expression failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int parse_while(struct parser *parser)
{
	if (backend_call(while_begin)(parser->scope->status))
		goto err_backend_failed;
	if (loop_condition_parse(parser))
		return 1;
	if (backend_call(while_cond)(parser->scope->status))
		goto err_backend_failed;
	if (loop_body_parse(parser))
		return 1;
	if (backend_call(while_end)(parser->scope->status))
		goto err_backend_failed;
	return 0;
err_backend_failed:
	printf("amc: parse_while: %lld,%lld: Backend call failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}
