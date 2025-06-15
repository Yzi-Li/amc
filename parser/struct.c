#include "include/constructor.h"
#include "include/expr.h"
#include "include/identifier.h"
#include "include/indent.h"
#include "include/keywords.h"
#include "include/struct.h"
#include "include/token.h"
#include "include/type.h"
#include "../include/backend.h"
#include "../include/token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int constructor_struct_elem(const char *se, struct file *f, void *data);
static int struct_def_read_elem(struct file *f, yz_struct *src,
		struct scope *scope);
static int struct_def_read_elems(struct file *f, yz_struct *src,
		struct scope *scope);
static int struct_def_read_name(struct file *f, str *name);
static int struct_def_read_start(struct file *f, const char *name);
static int struct_def_reg(yz_struct *src, struct scope *scope);
static int struct_def_reg_elem(yz_struct *src, struct symbol *elem);
static int struct_get_elem_handle_val(yz_val *val, struct symbol *elem);
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
	if ((expr = parse_expr(f, 1, handle->scope)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(expr, handle->scope) > 0)
		goto err_cannot_apply_expr;
	if ((val = identifier_expr_val_handle(&expr,
					&src->elems[handle->index]
					->result_type)) == NULL)
		goto err_cannot_apply_expr;
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

int struct_def_read_elem(struct file *f, yz_struct *src, struct scope *scope)
{
	int indent = 0;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line,
	    orig_pos = f->pos;
	struct symbol *elem = NULL;
	if ((indent = indent_read(f)) != scope->indent) {
		f->cur_column = orig_column;
		f->cur_line = orig_line;
		f->pos = orig_pos;
		return -1;
	}
	if (parse_comment(f))
		return 0;
	elem = calloc(1, sizeof(*elem));
	if (parse_type_name_pair(f, elem, scope))
		goto err_free_elem;
	if (struct_def_reg_elem(src, elem))
		goto err_free_elem;
	return keyword_end(f);
err_free_elem:
	free(elem);
	return 1;
}

int struct_def_read_elems(struct file *f, yz_struct *src, struct scope *scope)
{
	int orig_indent = scope->indent,
	    ret = 0;
	scope->indent += 1;
	while ((ret = struct_def_read_elem(f, src, scope)) != -1) {
		if (ret > 0)
			return 1;
	}
	scope->indent = orig_indent;
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

int struct_def_reg(yz_struct *src, struct scope *scope)
{
	scope->structures.count += 1;
	scope->structures.elems = realloc(scope->structures.elems,
			sizeof(*scope->structures.elems)
			* scope->structures.count);
	scope->structures.elems[scope->structures.count - 1] = src;
	return 0;
}

int struct_def_reg_elem(yz_struct *src, struct symbol *elem)
{
	src->elem_count += 1;
	src->elems = realloc(src->elems, sizeof(*src->elems)
			* src->elem_count);
	src->elems[src->elem_count - 1] = elem;
	return 0;
}

int struct_get_elem_handle_val(yz_val *val, struct symbol *elem)
{
	struct expr *expr = NULL;
	expr = malloc(sizeof(*expr));
	expr->vall = NULL;
	expr->valr = malloc(sizeof(*expr->valr));
	expr->valr->type = AMC_SYM;
	expr->valr->v = elem;
	expr->op = malloc(sizeof(*expr->op));
	expr->op->id = OP_EXTRACT_VAL;
	expr->op->priority = 0;
	expr->op->sym = NULL;
	expr->sum_type = &elem->result_type.type;
	val->type = AMC_EXPR;
	val->v = expr;
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
		if (name->len != src->elems[i]->name_len)
			continue;
		if (strncmp(name->s, src->elems[i]->name, name->len) == 0)
			return i;
	}
	return -1;
}

int constructor_struct(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct constructor_handle *handle = malloc(sizeof(*handle));
	handle->index = 0;
	handle->len = ((yz_struct*)sym->result_type.v)->elem_count;
	handle->scope = scope;
	handle->sym = sym;
	handle->vs = calloc(handle->len, sizeof(yz_val*));
	if (token_parse_list(",}", handle, f, constructor_struct_elem))
		goto err_free_handle;
	if (backend_call(struct_def)(&sym->backend_status, handle->vs,
				handle->len))
		goto err_backend_failed;
	constructor_handle_free(handle);
	if (f->src[f->pos] != '}')
		goto err_not_end;
	file_pos_next(f);
	file_skip_space(f);
	return keyword_end(f);
err_backend_failed:
	constructor_handle_free(handle);
	printf("amc: constructor_struct: %lld,%lld: Backend call failed!\n",
			f->cur_line, f->cur_column);
err_free_handle:
	constructor_handle_free(handle);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_not_end:
	printf("amc: constructor_struct: %lld,%lld: Not end!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_struct(struct file *f, struct symbol *sym, struct scope *scope)
{
	str name = TOKEN_NEW;
	yz_struct *result = NULL;
	int ret = 0;
	if ((ret = struct_def_read_name(f, &name)) > 0)
		return 1;
	result = calloc(1, sizeof(*result));
	result->name = str2chr(name.s, name.len);
	if (ret == -1)
		result->flags.rec = 1;
	if (struct_def_read_start(f, result->name))
		goto err_free_result;
	if (struct_def_read_elems(f, result, scope))
		goto err_free_result;
	if (struct_def_reg(result, scope))
		goto err_free_result;
	return 0;
err_free_result:
	free(result->name);
	free(result);
	return 1;
}

int parse_type_struct(str *token, yz_val *type, struct scope *scope)
{
	char *err_msg;
	type->type = YZ_STRUCT;
	if ((type->v = struct_type_find(token, scope)) == NULL)
		goto err_not_found;
	return 0;
err_not_found:
	err_msg = str2chr(token->s, token->len);
	printf("amc: parse_type_struct: Struct: '%s' not found!\n", err_msg);
	free(err_msg);
	return 1;
}

int struct_get_elem(struct file *f, yz_val *val, struct scope *scope)
{
	int ret = 0;
	struct symbol *sym = val->v;
	yz_struct *src = sym->result_type.v;
	str token = TOKEN_NEW;
	if (sym->result_type.type != YZ_STRUCT)
		return 1;
	if (struct_get_elem_read_name(f, &token))
		goto err_print_pos;
	if ((ret = struct_get_elem_read_elem(&token, src)) == -1)
		goto err_print_pos;
	if (backend_call(struct_get_elem)(sym->backend_status, src, ret))
		goto err_backend_failed;
	return struct_get_elem_handle_val(val, src->elems[ret]);
err_print_pos:
	printf("| struct_get_elem: %lld,%lld\n",
			f->cur_line, f->cur_column);
	return 1;
err_backend_failed:
	printf("amc: struct_get_elem: %lld,%lld: Backend call failed!\n",
			f->cur_line, f->cur_column);
	return 1;
}

yz_struct *struct_type_find(str *s, struct scope *scope)
{
	yz_struct *result = NULL;
	if ((result = struct_type_get(s, scope->structures.elems,
					scope->structures.count)) != NULL)
		return result;
	if (scope->parent != NULL)
		return struct_type_find(s, scope->parent);
	return NULL;
}

yz_struct *struct_type_get(str *s, yz_struct **structures, int count)
{
	for (int i = 0; i < count; i++) {
		if (s->len != strlen(structures[i]->name))
			continue;
		if (strncmp(s->s, structures[i]->name, s->len) == 0)
			return structures[i];
	}
	return NULL;
}
