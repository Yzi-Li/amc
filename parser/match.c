/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/block.h"
#include "include/expr.h"
#include "include/indent.h"
#include "include/keywords.h"
#include "include/utils.h"
#include "../include/backend.h"
#include "../include/parser.h"

enum MATCH_RESULT {
	MATCH_RESULT_HANDLED,
	MATCH_RESULT_END,
	MATCH_RESULT_FAULT
};

static enum MATCH_RESULT match_parse_case(struct parser *parser,
		backend_cond_match_handle *handle);
static yz_val *match_parse_case_cond(struct parser *parser);
static int match_parse_condition(struct parser *parser);

enum MATCH_RESULT match_parse_case(struct parser *parser,
		backend_cond_match_handle *handle)
{
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line,
	    orig_pos = parser->f->pos;
	yz_val *val = NULL;
	if (indent_read(parser->f) != parser->scope->indent)
		goto restore_end;
	if (parser->f->src[parser->f->pos] != '|')
		goto restore_end;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	if ((val = match_parse_case_cond(parser)) == NULL)
		return MATCH_RESULT_FAULT;
	if (backend_call(cond_match_case)(handle, val))
		return MATCH_RESULT_FAULT;
	if (parse_block(parser))
		return MATCH_RESULT_FAULT;
	if (backend_call(cond_match_case_end)(handle))
		return MATCH_RESULT_FAULT;
	return MATCH_RESULT_HANDLED;
restore_end:
	parser->f->cur_column = orig_column;
	parser->f->cur_line = orig_line;
	parser->f->pos = orig_pos;
	return MATCH_RESULT_END;
}

yz_val *match_parse_case_cond(struct parser *parser)
{
	struct expr *expr = NULL;
	yz_val *val = NULL;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	if ((expr = parse_expr(parser, 1)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(parser, expr) > 0)
		goto err_cannot_apply_expr;
	if ((val = expr2yz_val(expr)) == NULL)
		goto err_free_expr;
	return val;
err_cannot_parse_expr:
	printf("|< amc: %s: %lld,%lld: Cannot parse expression!\n",
			__func__, orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
err_cannot_apply_expr:
	printf("|< amc: %s: %lld,%lld: Cannot apply expression!\n",
			__func__, orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
err_free_expr:
	free_expr(expr);
	return NULL;
}

int match_parse_condition(struct parser *parser)
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
	printf("|< amc: %s: %lld,%lld: Cannot parse expression!\n",
			__func__, orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("|< amc: %s: %lld,%lld: Cannot apply expression!\n",
			__func__, orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_match(struct parser *parser)
{
	backend_cond_match_handle *handle = NULL;
	enum MATCH_RESULT ret = 0;
	if (match_parse_condition(parser))
		return 1;
	if (try_next_line(parser->f) != TRY_RESULT_HANDLED)
		goto err_no_nl;
	if ((handle = backend_call(cond_match_begin)()) == NULL)
		return 1;
	while ((ret = match_parse_case(parser, handle)) != MATCH_RESULT_END) {
		if (ret == MATCH_RESULT_FAULT)
			goto err_free_handle;
	}
	if (backend_call(cond_match_end)(handle))
		return 1;
	return 0;
err_no_nl:
	printf("amc: %s: %lld,%lld: Missing line break character.\n",
			__func__, parser->f->cur_line, parser->f->cur_column);
	return 1;
err_free_handle:
	backend_call(cond_match_free_handle)(handle);
	return 1;
}
