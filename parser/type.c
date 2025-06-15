#include "include/array.h"
#include "include/ptr.h"
#include "include/struct.h"
#include "include/token.h"
#include "include/type.h"
#include "../include/token.h"
#include <stdio.h>
#include <string.h>

static int type_pair_parse_name(struct file *f, str *name);
static int type_pair_parse_type(struct file *f, struct symbol *sym,
		struct scope *scope);

int type_pair_parse_name(struct file *f, str *name)
{
	if (token_read_before(SPECIAL_TOKEN_END, name, f) == NULL)
		return 1;
	if (strchr(" \t\n", f->src[f->pos]) != NULL)
		file_skip_space(f);
	return 0;
}

int type_pair_parse_type(struct file *f, struct symbol *sym,
		struct scope *scope)
{
	int ret = 0;
	if (f->src[f->pos] != ':')
		goto err_type_indicator_not_found;
	file_pos_next(f);
	file_skip_space(f);
	if (f->src[f->pos] == 'm'
			&& f->src[f->pos + 1] == 'u'
			&& f->src[f->pos + 2] == 't') {
		file_pos_nnext(3, f);
		file_skip_space(f);
		sym->flags.mut = 1;
	}
	if ((ret = parse_type(f, &sym->result_type, scope)) > 0)
		return 1;
	if (ret == -1)
		sym->flags.can_null = 1;
	return 0;
err_type_indicator_not_found:
	printf("amc: type_pair_parse_name: %lld,%lld: "
			"Type indicator not found\n",
			f->cur_line, f->cur_column);
	return 1;
}

int parse_type(struct file *f, yz_val *type, struct scope *scope)
{
	str token = TOKEN_NEW;
	if (type == NULL)
		goto err_type_null;
	if (f->src[f->pos] == '*') {
		return parse_type_ptr(f, type, scope);
	} else if (f->src[f->pos] == '[') {
		return parse_type_array(f, type, scope);
	}
	if (token_read_before(SPECIAL_TOKEN_END, &token, f) == NULL)
		return 1;
	file_skip_space(f);
	type->type = yz_type_get(&token);
	type->v = NULL;
	if (type->type == AMC_ERR_TYPE)
		return parse_type_struct(&token, type, scope);
	return 0;
err_type_null:
	printf("amc: parse_type: ARG is empty!\n");
	return 1;
}

int parse_type_name_pair(struct file *f, struct symbol *result,
		struct scope *scope)
{
	str name = TOKEN_NEW;
	if (type_pair_parse_name(f, &name))
		return 1;
	if (type_pair_parse_type(f, result, scope))
		return 1;
	result->name = str2chr(name.s, name.len);
	result->name_len = name.len;
	return 0;
}
