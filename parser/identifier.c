/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/array.h"
#include "include/block.h"
#include "include/enum.h"
#include "include/expr.h"
#include "include/identifier.h"
#include "include/indent.h"
#include "include/struct.h"
#include "include/symbol.h"
#include "include/token.h"
#include "include/utils.h"
#include "../include/backend.h"
#include "../include/checker/ptr.h"
#include "../include/checker/symbol.h"
#include "../include/checker/type.h"
#include "../include/expr.h"
#include "../include/parser.h"
#include "../include/ptr.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../utils/str/str.h"
#include <stdio.h>
#include <stdlib.h>

static int identifier_assign_backend_call(struct symbol *sym, yz_val *val,
		enum OP_ID mode);
static int identifier_read_enum(struct parser *parser, yz_val *val, str *name);
static enum TRY_RESULT identifier_try_handle_null(struct parser *parser,
		struct symbol *ident, yz_val *val);

int identifier_assign_backend_call(struct symbol *sym, yz_val *val,
		enum OP_ID mode)
{
	if (sym->flags.mut) {
		if (backend_call(var_set)(sym, val, mode))
			return 1;
	} else {
		if (mode != OP_ASSIGN)
			goto err_unsupport_op;
		if (backend_call(var_immut_init)(sym, val))
			return 1;
	}
	sym->flags.is_init = 1;
	return 0;
err_unsupport_op:
	printf("amc: identifier_assign_backend_call: "
			"Unsupport operator for immutable identifier!\n");
	return 1;
}

int identifier_read_enum(struct parser *parser, yz_val *val, str *name)
{
	char *err_msg;
	yz_enum_item *item = NULL;
	yz_enum *src = yz_enum_find(name, parser->scope);
	str token = TOKEN_NEW;
	if (src == NULL)
		goto err_enum_not_found;
	file_pos_next(parser->f);
	if (symbol_read(&token, parser->f))
		return 1;
	if ((item = yz_enum_item_find(&token, src)) == NULL)
		return 1;
	val->type = src->type;
	val->l = item->u;
	return 0;
err_enum_not_found:
	err_msg = str2chr(name->s, name->len);
	printf("amc: identifier_read_enum: %lld,%lld: Enum: '%s' not found!\n",
			parser->f->cur_line, parser->f->cur_column,
			err_msg);
	free(err_msg);
	return 1;
}

enum TRY_RESULT
identifier_try_handle_null(struct parser *parser, struct symbol *ident,
		yz_val *val)
{
	backend_null_handle *handle;
	str null_handler = {.len = 2, .s = "|?"};
	i64 orig_line = parser->f->cur_line,
	    orig_column = parser->f->cur_column,
	    orig_pos = parser->f->pos;
	if (!try_next_line(parser->f))
		return TRY_RESULT_FAULT;
	if (indent_read(parser->f) != parser->scope->indent)
		goto not_handled_restore_pos;
	if (!token_try_read(&null_handler, parser->f))
		goto err_maybe_null;
	if (!block_check_start(parser->f))
		return TRY_RESULT_FAULT;
	((yz_ptr_type*)ident->result_type.v)->flag_checked_null = 1;
	if (backend_call(null_handle_begin)(&handle, val))
		return TRY_RESULT_FAULT;
	if (parse_block(parser))
		return TRY_RESULT_FAULT;
	if (backend_call(null_handle_end)(handle))
		return TRY_RESULT_FAULT;
	return TRY_RESULT_HANDLED;
not_handled_restore_pos:
	parser->f->cur_line = orig_line;
	parser->f->cur_column = orig_column;
	parser->f->pos = orig_pos;
	return 0;
err_maybe_null:
	printf("amc: identifier_check_can_assign_val: %lld,%lld: "
			ERROR_STR":\n"
			"| Assign a can null value to a cannot be null "
			"identifier: '%s'.\n"
			"| "HINT_STR": "
			"Append '|? =>' to next line to handle null branch\n",
			parser->f->cur_line, parser->f->cur_column,
			ident->name.s);
	return TRY_RESULT_FAULT;
}

int identifier_assign_get_val(struct parser *parser,
		yz_type *dest_type, yz_val **result)
{
	struct expr *expr = NULL;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	if ((expr = parse_expr(parser, 1)) == NULL)
		return err_print_pos(__func__, "Cannot parse expr!",
				orig_line, orig_column);
	if (expr_apply(parser, expr) > 0)
		goto err_cannot_apply_expr;
	if ((*result = identifier_handle_expr_val(expr, dest_type)) == NULL)
		goto err_cannot_handle_expr;
	return 0;
err_cannot_apply_expr:
	free_expr(expr);
	return err_print_pos(__func__, "Cannot apply expr!",
			orig_line, orig_column);
err_cannot_handle_expr:
	free_expr(expr);
	return err_print_pos(__func__, "Cannot handle expr value!",
			orig_line, orig_column);
}

int identifier_assign_val(struct parser *parser, struct symbol *sym,
		enum OP_ID mode)
{
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	int ret = 0;
	yz_val *val = NULL;
	if (sym->result_type.type == YZ_PTR)
		((yz_ptr_type*)sym->result_type.v)->flag_checked_null = 0;
	if (!check_sym_can_assign(sym))
		return err_print_pos(__func__, NULL, orig_line, orig_column);
	if (identifier_assign_get_val(parser, &sym->result_type, &val))
		return 1;
	if (!(ret = identifier_check_can_assign_val(parser, sym, val)))
		return 1;
	if (identifier_assign_backend_call(sym, val, mode))
		return err_print_pos(__func__, "Backend call failed!",
				orig_line, orig_column);
	free_yz_val(val);
	if (parser->f->src[parser->f->pos] == ']')
		file_pos_next(parser->f);
	return 0;
}

int identifier_check_can_assign_val(struct parser *parser,
		struct symbol *ident, yz_val *val)
{
	yz_ptr_type *ptr = NULL,
	            *ident_ptr = ident->result_type.v;
	enum TRY_RESULT ret = 0;
	if (ident->result_type.type != YZ_PTR || ident_ptr->flag_can_null)
		return 1;
	if (val->type.type == YZ_NULL)
		return 0;
	if (val->type.type == AMC_EXPR && val->expr->op->id == OP_GET_ADDR) {
		if (check_ptr_get_addr_to_ident(val->expr, ident))
			goto err_print_pos;
	}
	if (val->type.type != AMC_SYM || val->sym->type != SYM_FUNC) {
		ptr = val->sym->result_type.v;
		if (!ptr->flag_can_null)
			return 1;
	}
	ret = identifier_try_handle_null(parser, ident, val);
	if (ret != TRY_RESULT_HANDLED)
		return 0;
	return 1;
err_print_pos:
	err_print_pos(__func__, NULL,
			parser->f->cur_line,
			parser->f->cur_column);
	return 0;
}

int identifier_check_mut(struct file *f)
{
	str expect = {.len = 3, .s = "mut"};
	if (token_try_read(&expect, f))
		return 1;
	return 0;
}

yz_val *identifier_handle_expr_val(struct expr *e, yz_type *type)
{
	yz_val *val = expr2yz_val(e);
	if (val == NULL)
		return NULL;
	if (identifier_handle_val_type(&val->type, type))
		return NULL;
	return val;
}

int identifier_handle_val_type(yz_type *src, yz_type *dest)
{
	if (!check_type_equal(src, dest))
		return 1;
	if (!YZ_IS_DIGIT(src->type) || !YZ_IS_DIGIT(dest->type))
		return 0;
	src->type = dest->type;
	src->v = dest->v;
	return 0;
}

int identifier_read(struct parser *parser, yz_val *val)
{
	char *err_msg;
	struct symbol *sym = NULL;
	str token = TOKEN_NEW;
	if (token_read_before(SPECIAL_TOKEN_END, &token, parser->f) == NULL)
		return 1;
	if (!symbol_find(&token, &sym, parser->scope, SYMG_SYM)) {
		if (parser->f->src[parser->f->pos] != '.')
			goto err_identifier_not_found;
		return identifier_read_enum(parser, val, &token);
	}
	val->v = sym;
	val->type.type = AMC_SYM;
	val->type.v = val->v;
	if (parser->f->src[parser->f->pos] == '[')
		return array_get_elem(parser, val);
	if (parser->f->src[parser->f->pos] == '.') {
		if (sym->result_type.type == YZ_PTR
				&& ((yz_ptr_type*)sym->result_type.v)
				->ref.type == YZ_STRUCT)
			return struct_get_elem_from_ptr(parser, val);
		if (sym->result_type.type != YZ_STRUCT)
			goto err_syntax_err;
		return struct_get_elem(parser, val);
	}
	file_skip_space(parser->f);
	return 0;
err_identifier_not_found:
	err_msg = str2chr(token.s, token.len);
	printf("amc: identifier_read: %lld,%lld: "
			"Identifier: '%s' not found!\n",
			parser->f->cur_line, parser->f->cur_column,
			err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_syntax_err:
	err_msg = str2chr(token.s, token.len);
	printf("amc: identifier_read: %lld,%lld: "
			"Identifier: '%s' not struct!\n",
			parser->f->cur_line, parser->f->cur_column,
			err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}
