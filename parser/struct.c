/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/constructor.h"
#include "include/expr.h"
#include "include/identifier.h"
#include "include/indent.h"
#include "include/keywords.h"
#include "include/struct.h"
#include "include/op.h"
#include "include/token.h"
#include "include/type.h"
#include "include/utils.h"
#include "../include/backend.h"
#include "../include/checker/struct.h"
#include "../include/checker/symbol.h"
#include "../include/parser.h"
#include "../include/ptr.h"
#include "../include/token.h"
#include <sctrie.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int constructor_struct_elem(const char *se, struct file *f, void *data);
static int struct_def_read_elem(struct parser *parser, yz_struct *src);
static int struct_def_read_elems(struct parser *parser, yz_struct *src);
static int struct_def_read_name(struct file *f, str *name);
static int struct_def_read_start(struct file *f, const char *name);
static int struct_def_reg(yz_struct *src, struct scope *scope);
static int struct_def_reg_elem(yz_struct *src, struct symbol *elem);
static int struct_get_elem_from_ptr_handle_val(yz_val *val, int index,
		struct symbol *elem);
static int struct_get_elem_handle_val(yz_val *val, int index,
		struct symbol *elem);
static int struct_get_elem_read_name(struct file *f, str *name);
/**
 * @return: elem index.
 */
static int struct_get_elem_read_elem(str *name, yz_struct *src);

int constructor_struct_elem(const char *se, struct file *f, void *data)
{
	struct constructor_handle *handle = data;
	yz_struct *src = handle->sym->result_type.v;
	struct expr *expr = NULL;
	yz_val *val = NULL;
	if (handle->index > src->elem_count - 1)
		goto err_too_many_elem;
	if ((expr = parse_expr(handle->parser, 1)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(handle->parser, expr) > 0)
		goto err_cannot_apply_expr;
	if ((val = identifier_handle_expr_val(expr,
					&src->elems[handle->index]
					->result_type)) == NULL)
		goto err_cannot_apply_expr;
	src->elems[handle->index]->flags.is_init = 1;
	handle->vs[handle->index] = val;
	handle->index += 1;
	return token_list_elem_end(',', f);
err_too_many_elem:
	printf("amc: constructor_struct_elem: %lld,%lld: "
			"Too many elements!\n",
			f->cur_line, f->cur_column);
	return 1;
err_cannot_parse_expr:
	printf("amc: constructor_struct_elem: %lld,%lld: "
			"Cannot parse expr!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("amc: constructor_struct_elem: %lld,%lld: "
			"Cannot apply expr!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int struct_def_read_elem(struct parser *parser, yz_struct *src)
{
	int indent = 0;
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line,
	    orig_pos = parser->f->pos;
	struct symbol *elem = NULL;
	if ((indent = indent_read(parser->f)) != parser->scope->indent) {
		parser->f->cur_column = orig_column;
		parser->f->cur_line = orig_line;
		parser->f->pos = orig_pos;
		return -1;
	}
	if (parse_comment(parser->f))
		return 0;
	elem = calloc(1, sizeof(*elem));
	elem->type = SYM_STRUCT_ELEM;
	elem->flags.mut = identifier_check_mut(parser->f);
	if (parse_type_name_pair(parser, &elem->name, &elem->result_type))
		goto err_free_elem;
	if (struct_def_reg_elem(src, elem))
		goto err_free_elem;
	return keyword_end(parser->f);
err_free_elem:
	free(elem);
	return 1;
}

int struct_def_read_elems(struct parser *parser, yz_struct *src)
{
	int orig_indent = parser->scope->indent,
	    ret = 0;
	parser->scope->indent += 1;
	while ((ret = struct_def_read_elem(parser, src)) != -1) {
		if (ret > 0)
			return 1;
	}
	parser->scope->indent = orig_indent;
	return 0;
}

int struct_def_read_name(struct file *f, str *name)
{
	if (token_next(name, f))
		return 1;
	if (name->len == 3 && strncmp("rec", name->s, name->len) == 0) {
		name->len = 0;
		if (token_next(name, f))
			return 1;
		return -1;
	}
	return 0;
}

int struct_def_read_start(struct file *f, const char *name)
{
	str token = TOKEN_NEW;
	if (token_next(&token, f))
		return 1;
	if (token.len == 2 && strncmp("=>", token.s, token.len) == 0)
		return keyword_end(f);
	printf("amc: struct_def_read_start: %lld,%lld: "
			"Struct: '%s' define start character not found!\n",
			f->cur_line, f->cur_column,
			name);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int struct_def_reg(yz_struct *self, struct scope *scope)
{
	yz_user_type *type = sctrie_append_elem(&scope->types, sizeof(*type),
			self->name.s, self->name.len);
	if (type == NULL)
		goto err_defined;
	type->type = YZ_STRUCT;
	type->struct_ = self;
	return 0;
err_defined:
	printf("amc: struct_def_reg: "
			"Type defined: '%s'\n", self->name.s);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int struct_def_reg_elem(yz_struct *src, struct symbol *elem)
{
	src->elem_count += 1;
	src->elems = realloc(src->elems, sizeof(*src->elems)
			* src->elem_count);
	src->elems[src->elem_count - 1] = elem;
	return 0;
}

int struct_get_elem_from_ptr_handle_val(yz_val *val, int index,
		struct symbol *elem)
{
	yz_extract_val *v = malloc(sizeof(*v));
	v->index = index;
	v->sym = val->sym;
	v->elem = elem;
	v->type = YZ_EXTRACT_STRUCT_FROM_PTR;
	val->v = op_extract_val_expr_create(&elem->result_type, v);
	if (val->v == NULL)
		return 1;
	val->type.type = AMC_EXPR;
	val->type.v = val->v;
	return 0;
}

int struct_get_elem_handle_val(yz_val *val, int index, struct symbol *elem)
{
	yz_extract_val *v = malloc(sizeof(*v));
	v->index = index;
	v->sym = val->v;
	v->elem = elem;
	v->type = YZ_EXTRACT_STRUCT;
	val->v = op_extract_val_expr_create(&elem->result_type, v);
	if (val->v == NULL)
		return 1;
	val->type.type = AMC_EXPR;
	val->type.v = val->v;
	return 0;
}

int struct_get_elem_read_name(struct file *f, str *name)
{
	if (f->src[f->pos] != '.')
		return 1;
	file_pos_next(f);
	if (!file_try_skip_space(f))
		return 1;
	if (token_read_before(SPECIAL_TOKEN_END, name, f) == NULL)
		return 1;
	// TODO: Sub-structure.
	if (strchr(" \t\n", f->src[f->pos]) == NULL)
		return 1;
	file_skip_space(f);
	return 0;
}

int struct_get_elem_read_elem(str *name, yz_struct *src)
{
	for (int i = 0; i < src->elem_count; i++) {
		if (name->len != src->elems[i]->name.len)
			continue;
		if (strncmp(name->s, src->elems[i]->name.s, name->len) == 0)
			return i;
	}
	return -1;
}

int constructor_struct(struct parser *parser, struct symbol *sym)
{
	struct constructor_handle *handle = malloc(sizeof(*handle));
	handle->index = 0;
	handle->len = ((yz_struct*)sym->result_type.v)->elem_count;
	handle->parser = parser;
	handle->sym = sym;
	handle->vs = calloc(handle->len, sizeof(yz_val*));
	if (token_parse_list(",}", handle, parser->f, constructor_struct_elem))
		goto err_free_handle;
	if (backend_call(struct_def)(&sym->backend_status, handle->vs,
				handle->len))
		goto err_backend_failed;
	free_constructor_handle(handle);
	if (parser->f->src[parser->f->pos] != '}')
		goto err_not_end;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	return keyword_end(parser->f);
err_backend_failed:
	free_constructor_handle(handle);
	printf("amc: constructor_struct: %lld,%lld: Backend call failed!\n",
			parser->f->cur_line, parser->f->cur_column);
err_free_handle:
	free_constructor_handle(handle);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_not_end:
	printf("amc: constructor_struct: %lld,%lld: Not end!\n",
			parser->f->cur_line, parser->f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_struct(struct parser *parser)
{
	str name = TOKEN_NEW;
	yz_struct *result = NULL;
	int ret = 0;
	if ((ret = struct_def_read_name(parser->f, &name)) > 0)
		return 1;
	result = calloc(1, sizeof(*result));
	str_copy(&name, &result->name);
	if (ret == -1)
		result->flags.rec = 1;
	if (struct_def_read_start(parser->f, result->name.s))
		goto err_free_result;
	if (struct_def_read_elems(parser, result))
		goto err_free_result;
	if (struct_def_reg(result, parser->stat.has_pub
				? parser->scope_pub
				: parser->scope))
		goto err_free_result;
	return 0;
err_free_result:
	str_free_noself(&result->name);
	free(result);
	return 1;
}

int parse_type_struct(str *token, yz_type *result, struct scope *scope)
{
	char *err_msg;
	result->type = YZ_STRUCT;
	if ((result->v = struct_type_find(token, scope)) == NULL)
		goto err_not_found;
	return 0;
err_not_found:
	err_msg = str2chr(token->s, token->len);
	printf("amc: parse_type_struct: Struct: '%s' not found!\n", err_msg);
	free(err_msg);
	return 1;
}

int struct_get_elem(struct parser *parser, yz_val *val)
{
	int ret = 0;
	struct symbol *sym = val->v;
	yz_struct *src = sym->result_type.v;
	str token = TOKEN_NEW;
	if (struct_get_elem_read_name(parser->f, &token))
		goto err_print_pos;
	if ((ret = struct_get_elem_read_elem(&token, src)) == -1)
		goto err_print_pos;
	return struct_get_elem_handle_val(val, ret, src->elems[ret]);
err_print_pos:
	return err_print_pos(__func__, NULL, parser->f->cur_line,
			parser->f->cur_column);
}

int struct_get_elem_from_ptr(struct parser *parser, yz_val *val)
{
	int ret = 0;
	struct symbol *sym = val->v;
	yz_struct *src = ((yz_ptr_type*)sym->result_type.v)->ref.v;
	str token = TOKEN_NEW;
	if (struct_get_elem_read_name(parser->f, &token))
		goto err_print_pos;
	if ((ret = struct_get_elem_read_elem(&token, src)) == -1)
		goto err_print_pos;
	return struct_get_elem_from_ptr_handle_val(val, ret, src->elems[ret]);
err_print_pos:
	return err_print_pos(__func__, NULL, parser->f->cur_line,
			parser->f->cur_column);
}

int struct_set_elem(struct parser *parser, struct symbol *sym, int index,
		enum OP_ID mode)
{
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	yz_val *val = NULL;
	struct symbol *elem = ((yz_struct*)sym->result_type.v)->elems[index];
	if (elem->result_type.type == YZ_PTR)
		((yz_ptr_type*)elem->result_type.v)->flag_checked_null = 0;
	if (!check_struct_elem_can_assign(sym, elem))
		return err_print_pos(__func__, NULL, orig_line, orig_column);
	if (identifier_assign_get_val(parser, &elem->result_type, &val))
		return 1;
	if (!check_sym_can_assign_val(elem, val))
		goto err_cannot_assign;
	if (backend_call(struct_set_elem)(sym, index, val, mode))
		goto err_backend_failed;
	free_yz_val(val);
	return 0;
err_cannot_assign:
	free_yz_val(val);
	return err_print_pos(__func__, NULL, orig_line, orig_column);
err_backend_failed:
	free_yz_val(val);
	return err_print_pos(__func__, "Backend call failed!",
			orig_line, orig_column);
}

int struct_set_elem_from_ptr(struct parser *parser, struct symbol *sym,
		int index, enum OP_ID mode)
{
	i64 orig_column = parser->f->cur_column,
	    orig_line = parser->f->cur_line;
	yz_val *val = NULL;
	struct symbol *elem = ((yz_struct*)sym->result_type.v)->elems[index];
	if (elem->result_type.type == YZ_PTR)
		((yz_ptr_type*)elem->result_type.v)->flag_checked_null = 0;
	if (!check_struct_elem_can_assign(sym, elem))
		return err_print_pos(__func__, NULL, orig_line, orig_column);
	if (identifier_assign_get_val(parser, &elem->result_type, &val))
		return 1;
	if (!check_sym_can_assign_val(elem, val))
		return err_print_pos(__func__, NULL, orig_line, orig_column);
	if (backend_call(struct_set_elem_from_ptr)(sym, index, val, mode))
		return err_print_pos(__func__, "Backend call failed!",
				orig_line, orig_column);
	free_yz_val(val);
	return 0;
}

yz_struct *struct_type_find(str *s, struct scope *scope)
{
	struct yz_user_type *type = yz_user_type_find(s, scope);
	if (!type || type->type != YZ_STRUCT)
		return NULL;
	return type->struct_;
}
