#include "include/array.h"
#include "include/ptr.h"
#include "include/struct.h"
#include "include/token.h"
#include "include/type.h"
#include "../include/parser.h"
#include "../include/token.h"
#include <stdio.h>
#include <string.h>

static int type_get_from_module(str *name, yz_val *type,
		struct parser *parser);
static int type_pair_parse_name(struct file *f, str *name);
static int type_pair_parse_type(struct parser *parser, struct symbol *sym);

int type_get_from_module(str *name, yz_val *type, struct parser *parser)
{
	yz_module *mod = NULL;
	struct scope *orig_scope = parser->scope;
	int ret = 0;
	if (parser->f->src[parser->f->pos] != '.')
		return 1;
	if ((mod = parser_imported_find(&parser->imported, name)) == NULL)
		return 1;
	file_pos_next(parser->f);
	if (!file_try_skip_space(parser->f))
		goto err_has_space;
	parser->scope = mod->scope;
	ret = parse_type(parser, type);
	parser->scope = orig_scope;
	return ret;
err_has_space:
	printf("amc: type_get_from_module: %lld,%lld: Has space!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int type_pair_parse_name(struct file *f, str *name)
{
	if (token_read_before(SPECIAL_TOKEN_END, name, f) == NULL)
		return 1;
	if (strchr(" \t\n", f->src[f->pos]) != NULL)
		file_skip_space(f);
	return 0;
}

int type_pair_parse_type(struct parser *parser, struct symbol *sym)
{
	int ret = 0;
	if (parser->f->src[parser->f->pos] != ':')
		goto err_type_indicator_not_found;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	if (parser->f->src[parser->f->pos] == 'm'
			&& parser->f->src[parser->f->pos + 1] == 'u'
			&& parser->f->src[parser->f->pos + 2] == 't') {
		file_pos_nnext(3, parser->f);
		file_skip_space(parser->f);
		sym->flags.mut = 1;
	}
	if ((ret = parse_type(parser, &sym->result_type)) > 0)
		return 1;
	if (ret == -1)
		sym->flags.can_null = 1;
	return 0;
err_type_indicator_not_found:
	printf("amc: type_pair_parse_name: %lld,%lld: "
			"Type indicator not found\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int parse_type(struct parser *parser, yz_val *type)
{
	char *err_msg = NULL;
	str token = TOKEN_NEW;
	if (type == NULL)
		goto err_type_null;
	if (parser->f->src[parser->f->pos] == '*') {
		return parse_type_ptr(parser, type);
	} else if (parser->f->src[parser->f->pos] == '[') {
		return parse_type_array(parser, type);
	}
	if (token_read_before(SPECIAL_TOKEN_END, &token, parser->f) == NULL)
		return 1;
	if (parser->f->src[parser->f->pos] == '.') {
		if (type_get_from_module(&token, type, parser))
			goto err_unknown_type;
		return 0;
	}
	file_skip_space(parser->f);
	type->type = yz_type_get(&token);
	type->v = NULL;
	if (type->type != AMC_ERR_TYPE)
		return 0;
	if (parse_type_struct(&token, type, parser->scope))
		goto err_get_type_failed;
	return 0;
err_type_null:
	printf("amc: parse_type: %lld,%lld: ARG is empty!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
err_get_type_failed:
	err_msg = str2chr(token.s, token.len);
	printf("amc; parse_type: %lld,%lld: Try get type: '%s' failed!\n",
			parser->f->cur_line, parser->f->cur_column, err_msg);
	free(err_msg);
	return 1;
err_unknown_type:
	err_msg = str2chr(token.s, token.len);
	printf("| amc; parse_type: %lld,%lld: Unknown type: '%s'\n",
			parser->f->cur_line, parser->f->cur_column, err_msg);
	free(err_msg);
	return 1;
}

int parse_type_name_pair(struct parser *parser, struct symbol *result)
{
	str name = TOKEN_NEW;
	if (type_pair_parse_name(parser->f, &name))
		return 1;
	if (type_pair_parse_type(parser, result))
		return 1;
	result->name = str2chr(name.s, name.len);
	result->name_len = name.len;
	return 0;
}
