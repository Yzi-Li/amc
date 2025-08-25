/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/block.h"
#include "include/expr.h"
#include "include/indent.h"
#include "include/keywords.h"
#include "include/utils.h"
#include "../include/backend.h"
#include "../include/enum.h"
#include "../include/parser.h"
#include "../utils/utils.h"
#include <stdio.h>

enum MATCH_RESULT {
	MATCH_RESULT_HANDLED,
	MATCH_RESULT_END,
	MATCH_RESULT_FAULT
};

enum MATCH_MODE {
	MATCH_MODE_FAULT = -1,
	MATCH_MODE_ENUM
};

struct match_context_enum {
	yz_enum *self;
	unsigned int enum_count;
};

union match_context_data {
	struct match_context_enum e;
	void *v;
};

struct match_context {
	union match_context_data data;
	backend_cond_match_handle *handle;
	enum MATCH_MODE mode;
	struct parser *parser;
};

static int match_end(struct match_context *context);
static int match_end_enum_mode(struct match_context *context);
static enum MATCH_MODE match_get_mode(yz_val *val);
static int match_handle_case_context(struct match_context *context,
		yz_val *val);
static int match_handle_case_enum(struct match_context *context, yz_val *val);
static int match_handle_context(struct match_context *context, yz_val *val);
static int match_handle_enum_mode(struct match_context *context, yz_val *val);
static enum MATCH_RESULT match_parse_case(struct match_context *context);
static yz_val *match_parse_case_cond(struct parser *parser);
static yz_val *match_parse_condition(struct parser *parser);

int match_end(struct match_context *context)
{
	switch (context->mode) {
	case MATCH_MODE_ENUM:
		return match_end_enum_mode(context);
		break;
	default: break;
	}
	return 1;
}

int match_end_enum_mode(struct match_context *context)
{
	if (context->data.e.enum_count != context->data.e.self->count)
		goto err_must_match_all;
	return 0;
err_must_match_all:
	printf("amc: %s: %lld,%lld: "ERROR_STR":\n"
			"| You must match all cases "
			"of the enum: '%s'\n",
			__func__,
			context->parser->f->cur_line,
			context->parser->f->cur_column,
			context->data.e.self->name.s);
	return 1;
}

enum MATCH_MODE match_get_mode(yz_val *val)
{
	if (val->type.type != AMC_SYM)
		return MATCH_MODE_FAULT;
	if (val->data.sym->result_type.type == YZ_ENUM)
		return MATCH_MODE_ENUM;
	return MATCH_MODE_FAULT;
}

int match_handle_case_context(struct match_context *context, yz_val *val)
{
	switch (context->mode) {
	case MATCH_MODE_ENUM:
		return match_handle_case_enum(context, val);
		break;
	default: break;
	}
	return 1;
}

int match_handle_case_enum(struct match_context *context, yz_val *val)
{
	if (val->type.type != YZ_ENUM_ITEM)
		goto err_not_enum;
	if (val->type.v != context->data.e.self)
		goto err_not_same_enum;
	context->data.e.enum_count += 1;
	return 0;
err_not_enum:
	printf("amc: %s: %lld,%lld: "ERROR_STR":\n"
			"| Match enum but case not enum!\n",
			__func__,
			context->parser->f->cur_line,
			context->parser->f->cur_column);
	return 1;
err_not_same_enum:
	printf("amc: %s: %lld,%lld: "ERROR_STR":\n"
			"| Match enum and case enum is different!\n",
			__func__,
			context->parser->f->cur_line,
			context->parser->f->cur_column);
	return 1;
}

int match_handle_context(struct match_context *context, yz_val *val)
{
	if ((context->mode = match_get_mode(val)) == MATCH_MODE_FAULT)
		return 1;
	switch (context->mode) {
	case MATCH_MODE_ENUM:
		return match_handle_enum_mode(context, val);
		break;
	default: break;
	}
	return 1;
}

int match_handle_enum_mode(struct match_context *context, yz_val *val)
{
	if (val->type.type != AMC_SYM)
		return 1;
	if (val->data.sym->result_type.type != YZ_ENUM)
		return 1;
	context->data.e.enum_count = 0;
	context->data.e.self = val->data.sym->result_type.v;
	return 0;
}

enum MATCH_RESULT
match_parse_case(struct match_context *context)
{
	i64 orig_column = context->parser->f->cur_column,
	    orig_line = context->parser->f->cur_line,
	    orig_pos = context->parser->f->pos;
	yz_val *val = NULL;
	if (indent_read(context->parser->f) != context->parser->scope->indent)
		goto restore_end;
	if (context->parser->f->src[context->parser->f->pos] != '|')
		goto restore_end;
	file_pos_next(context->parser->f);
	file_skip_space(context->parser->f);
	if ((val = match_parse_case_cond(context->parser)) == NULL)
		return MATCH_RESULT_FAULT;
	if (match_handle_case_context(context, val))
		return MATCH_RESULT_FAULT;
	if (backend_call(cond_match_case)(context->handle, val))
		return MATCH_RESULT_FAULT;
	if (parse_block(context->parser))
		return MATCH_RESULT_FAULT;
	if (backend_call(cond_match_case_end)(context->handle))
		return MATCH_RESULT_FAULT;
	return MATCH_RESULT_HANDLED;
restore_end:
	context->parser->f->cur_column = orig_column;
	context->parser->f->cur_line = orig_line;
	context->parser->f->pos = orig_pos;
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

yz_val *match_parse_condition(struct parser *parser)
{
	struct expr *expr = NULL;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	if ((expr = parse_expr(parser, 1)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(parser, expr) > 0)
		goto err_cannot_apply_expr;
	return expr2yz_val(expr);
err_cannot_parse_expr:
	printf("|< amc: %s: %lld,%lld: Cannot parse expression!\n",
			__func__, orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
err_cannot_apply_expr:
	printf("|< amc: %s: %lld,%lld: Cannot apply expression!\n",
			__func__, orig_line, orig_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
}

int parse_match(struct parser *parser)
{
	enum MATCH_RESULT ret = 0;
	yz_val *val = NULL;
	struct match_context context = {
		.data.v = NULL,
		.parser = parser
	};
	if ((val = match_parse_condition(context.parser)) == NULL)
		return 1;
	if (match_handle_context(&context, val))
		goto err_free_val;
	if (try_next_line(parser->f) != TRY_RESULT_HANDLED)
		goto err_no_nl;
	if ((context.handle = backend_call(cond_match_begin)(val)) == NULL)
		goto err_free_val;
	free_yz_val(val);
	while ((ret = match_parse_case(&context))
			!= MATCH_RESULT_END) {
		if (ret == MATCH_RESULT_FAULT)
			goto err_free_handle;
	}
	if (match_end(&context))
		goto err_free_handle;
	if (backend_call(cond_match_end)(context.handle))
		return 1;
	return 0;
err_no_nl:
	printf("amc: %s: %lld,%lld: Missing line break character.\n",
			__func__, parser->f->cur_line, parser->f->cur_column);
err_free_val:
	free_yz_val(val);
	return 1;
err_free_handle:
	backend_call(cond_match_free_handle)(context.handle);
	return 1;
}
