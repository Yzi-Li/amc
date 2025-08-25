/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/block.h"
#include "include/expr.h"
#include "include/indent.h"
#include "include/keywords.h"
#include "../include/backend.h"
#include "../include/parser.h"
#include "../include/token.h"
#include <stdio.h>

enum IF_CONTEXT_RESULT {
	IF_RESULT_CONTINUE,
	IF_RESULT_END,
	IF_RESULT_FAULT
};

struct if_context {
	backend_cond_if_handle *handle;
	struct parser *parser;
};

static int if_condition_parse(struct parser *parser);
static enum IF_CONTEXT_RESULT if_continue(struct if_context *context);
static int if_parse_block(struct parser *parser);
static int parse_elif(struct if_context *context);
static int parse_else(struct if_context *context);

int if_condition_parse(struct parser *parser)
{
	struct expr *expr = NULL;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	if ((expr = parse_expr(parser, 1)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(parser, expr) > 0)
		goto err_cannot_apply_expr;
	free_expr(expr);
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

enum IF_CONTEXT_RESULT if_continue(struct if_context *context)
{
	str elif_str = {.len = 4, .s = "elif"},
	    else_str = {.len = 4, .s = "else"};
	i64 orig_column = context->parser->f->cur_column,
	    orig_line = context->parser->f->cur_line,
	    orig_pos = context->parser->f->pos;
	if (indent_read(context->parser->f) != context->parser->scope->indent)
		goto restore_end;
	if (token_try_read(&elif_str, context->parser->f)) {
		if (parse_elif(context))
			return IF_RESULT_FAULT;
		return IF_RESULT_CONTINUE;
	}
	if (token_try_read(&else_str, context->parser->f)) {
		if (parse_else(context))
			return IF_RESULT_FAULT;
		return IF_RESULT_END;
	}
	if (parse_comment(context->parser->f))
		return IF_RESULT_CONTINUE;
restore_end:
	context->parser->f->cur_column = orig_column;
	context->parser->f->cur_line = orig_line;
	context->parser->f->pos = orig_pos;
	return IF_RESULT_END;
}

int if_parse_block(struct parser *parser)
{
	if (parse_block(parser))
		goto err_parse_block_failed;
	return 0;
err_parse_block_failed:
	printf("amc: if_block_parse: %lld,%lld: Parse block failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int parse_elif(struct if_context *context)
{
	if (if_condition_parse(context->parser))
		return 1;
	if (backend_call(cond_if_cond)(context->handle))
		return 1;
	if (if_parse_block(context->parser))
		return 1;
	if (backend_call(cond_elif)(context->handle))
		goto err_backend_failed;
	return 0;
err_backend_failed:
	printf("amc: parse_elif: %lld,%lld: Backend call failed!\n",
			context->parser->f->cur_line,
			context->parser->f->cur_column);
	return 1;
}

int parse_else(struct if_context *context)
{
	str block_start = {.len = 2, .s = "=>"};
	if (!token_try_read(&block_start, context->parser->f))
		goto err_block_start_not_found;
	if (parse_block(context->parser))
		goto err_parse_block_failed;
	if (backend_call(cond_else)(context->handle))
		goto err_backend_failed;
	return 0;
err_block_start_not_found:
	printf("amc: parse_else: %lld,%lld: block start symbol not found!\n",
			context->parser->f->cur_line,
			context->parser->f->cur_column);
	return 1;
err_parse_block_failed:
	printf("amc: parse_else: %lld,%lld: Parse block failed!\n",
			context->parser->f->cur_line,
			context->parser->f->cur_column);
	return 1;
err_backend_failed:
	printf("amc: parse_else: %lld,%lld: Backend call failed!\n",
			context->parser->f->cur_line,
			context->parser->f->cur_column);
	return 1;
}

int parse_if(struct parser *parser)
{
	struct if_context context = {
		.handle = backend_call(cond_if_begin)(),
		.parser = parser
	};
	int ret = 0;
	if (context.handle == NULL)
		goto err_backend_failed;
	if (if_condition_parse(parser))
		goto err_free_handle;
	if (backend_call(cond_if_cond)(context.handle))
		return 1;
	if (if_parse_block(parser))
		goto err_free_handle;
	if (backend_call(cond_if)(context.handle))
		goto err_free_handle;
	while ((ret = if_continue(&context)) != IF_RESULT_END) {
		if (ret == IF_RESULT_FAULT)
			goto err_free_handle;
	}
	if (backend_call(cond_if_end)(context.handle))
		return 1;
	return 0;
err_backend_failed:
	printf("amc: parse_if: %lld,%lld: Backend call failed!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
err_free_handle:
	backend_call(cond_if_free_handle)(context.handle);
	return 1;
}
