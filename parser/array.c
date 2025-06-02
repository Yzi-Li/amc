//TODO: Mutable array
#include "include/array.h"
#include "include/expr.h"
#include "include/identifier.h"
#include "include/keywords.h"
#include "include/type.h"
#include "../include/array.h"
#include "../include/backend.h"
#include "../include/token.h"

struct structure_array_handle {
	int index;
	int len;
	struct scope *scope;
	struct symbol *sym;
	yz_val **vs;
};

static int array_get_elem_handle_val(yz_val *val);
static int array_get_len(struct file *f, yz_array *arr);
static int array_get_len_check_mut(struct file *f);
static int array_get_len_end(struct file *f, int len);
static int array_get_len_from_num(struct file *f, int *len);
static void array_handle_free(struct structure_array_handle *handle);
static yz_val *array_read_offset(struct file *f, struct scope *scope);

int array_get_elem_handle_val(yz_val *val)
{
	struct expr *expr = NULL;
	struct symbol *sym = val->v;
	expr = malloc(sizeof(*expr));
	expr->vall = NULL;
	expr->valr = malloc(sizeof(*expr->valr));
	expr->valr->type = AMC_SYM;
	expr->valr->v = val->v;
	expr->op = malloc(sizeof(*expr->op));
	expr->op->id = OP_EXTRACT_VAL;
	expr->op->priority = 0;
	expr->op->sym = NULL;
	expr->sum_type = &((yz_array*)sym->result_type.v)->type.type;
	val->type = AMC_EXPR;
	val->v = expr;
	return 0;
}

int array_get_len(struct file *f, yz_array *arr)
{
	arr->len = 0;
	if (CHR_IS_NUM(f->src[f->pos]))
		return array_get_len_from_num(f, &arr->len);
	if (array_get_len_check_mut(f))
		return array_get_len_end(f, -1);
	return 0;
}

int array_get_len_check_mut(struct file *f)
{
	if (strncmp("mut", &f->src[f->pos], 3) != 0)
		return 0;
	file_pos_nnext(3, f);
	file_skip_space(f);
	printf("amc: array_get_len_check_mut: %lld,%lld: "
			"Mutable array is unsupport!\n",
			f->cur_line, f->cur_column);
	exit(1);
	return 1;
}

int array_get_len_end(struct file *f, int len)
{
	file_skip_space(f);
	if (f->src[f->pos] != ']')
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	if (len == -1)
		return 0;
	if (len == 0)
		goto err_len_must_be_specified;
	return 0;
err_len_must_be_specified:
	printf("amc: yz_array_get_len: Length must be specified and not '0'!\n"
			"| If you want a mutable array, please use 'mut'.\n"
			"| Or you can use an identifier to specify "
				"the length.\n");
	return 1;
}

int array_get_len_from_num(struct file *f, int *len)
{
	while (f->src[f->pos] != ']') {
		if (strchr(" \t\n", f->src[f->pos]) != NULL)
			return array_get_len_end(f, *len);
		if (parse_comment(f))
			return array_get_len_end(f, *len);
		if (!CHR_IS_NUM(f->src[f->pos]))
			goto err_not_num;
		*len *= 10;
		*len += (f->src[f->pos] - '0');
		file_pos_next(f);
	}
	return array_get_len_end(f, *len);
err_not_num:
	printf("amc: yz_array_get_len: Length character isn't number!\n");
	return 1;
}

void array_handle_free(struct structure_array_handle *handle)
{
	for (int i = 0; i < handle->len; i++) {
		if (handle->vs[handle->index] == NULL)
			return;
		expr_free_val(handle->vs[handle->index]);
	}
	free(handle);
}

yz_val *array_read_offset(struct file *f, struct scope *scope)
{
	yz_val type = {.type = YZ_U64, .l = 0};
	struct expr *expr = NULL;
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_read_offset_failed;
	if (expr_apply(expr, scope) > 0)
		goto err_read_offset_failed;
	return identifier_expr_val_handle(&expr, &type);
err_read_offset_failed:
	printf("amc: array_read_offset: %lld,%lld: Read offset failed.\n",
			orig_line, orig_column);
	return NULL;
}

int array_get_elem(struct file *f, yz_val *val, struct scope *scope)
{
	struct symbol *sym = val->v;
	char *name = NULL;
	yz_val *offset = NULL;
	file_pos_next(f);
	file_skip_space(f);
	if ((offset = array_read_offset(f, scope)) == NULL)
		return 1;
	if (f->src[f->pos] != ']')
		goto err_not_end;
	file_pos_next(f);
	file_skip_space(f);
	name = str2chr(sym->name, sym->name_len);
	if (backend_call(array_get_elem)(name, offset))
		goto err_backend_failed;
	free(name);
	return array_get_elem_handle_val(val);
err_backend_failed:
	free(name);
	printf("amc: array_get_elem: Backend call failed!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_not_end:
	printf("amc: array_get_elem: Not end!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int array_structure(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct structure_array_handle *handle = malloc(sizeof(*handle));
	char *name = NULL;
	handle->index = 0;
	handle->len = ((yz_array*)sym->result_type.v)->len;
	handle->scope = scope;
	handle->sym = sym;
	handle->vs = calloc(handle->len, sizeof(yz_val));
	if (token_parse_list(",}", handle, f, array_structure_elem))
		goto err_free_handle;
	name = str2chr(sym->name, sym->name_len);
	if (backend_call(array_def)(name, handle->vs, handle->len,
				scope->status))
		goto err_backend_failed;
	array_handle_free(handle);
	if (f->src[f->pos] != '}')
		goto err_not_end;
	file_pos_next(f);
	file_skip_space(f);
	return keyword_end(f);
err_backend_failed:
	array_handle_free(handle);
	printf("amc: array_structure: %lld,%lld: Backend call failed!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
err_free_handle:
	array_handle_free(handle);
	return 1;
err_not_end:
	printf("amc: array_structure: %lld,%lld: Not end!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int array_structure_elem(const char *se, struct file *f, void *data)
{
	struct structure_array_handle *handle = data;
	struct yz_array *arr = handle->sym->result_type.v;
	struct expr *expr = NULL;
	yz_val *val = NULL;
	if (handle->index > arr->len - 1)
		goto err_too_many_elem;
	if ((expr = parse_expr(f, 1, handle->scope)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(expr, handle->scope) > 0)
		goto err_cannot_apply_expr;
	if ((val = identifier_expr_val_handle(&expr, &arr->type)) == NULL)
		goto err_cannot_apply_expr;
	handle->vs[handle->index] = val;
	handle->index += 1;
	return token_list_elem_end(',', f);
err_too_many_elem:
	printf("amc: let_structure_array_elem: %lld,%lld: "
			"Too many elements!\n",
			f->cur_line, f->cur_column);
	return 1;
err_cannot_parse_expr:
	printf("amc: let_structure_array_elem: %lld,%lld: "
			"Cannot parse expr!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("amc: let_structure_array_elem: %lld,%lld: "
			"Cannot apply expr!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_type_array(struct file *f, yz_val *type)
{
	yz_array *arr = NULL;
	int ret = 0;
	file_pos_next(f);
	file_skip_space(f);
	arr = malloc(sizeof(*arr));
	type->type = YZ_ARRAY;
	type->v = arr;
	if ((ret = parse_type(f, &arr->type)) > 0)
		goto err_free_arr;
	if (f->src[f->pos] != ',')
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	return array_get_len(f, arr);
err_free_arr:
	free(arr);
	type->v = NULL;
	return 1;
}
