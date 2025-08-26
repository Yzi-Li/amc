/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/array.h"
#include "include/ptr.h"
#include "include/symbol.h"
#include "include/token.h"
#include "include/type.h"
#include "../include/parser.h"
#include "../include/token.h"
#include <sctrie.h>
#include <stdio.h>
#include <string.h>

static int type_get_from_module(str *name, yz_type *type,
		struct parser *parser);
static int type_pair_parse_type(struct parser *parser, yz_type *type);

int type_get_from_module(str *name, yz_type *type, struct parser *parser)
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

int type_pair_parse_type(struct parser *parser, yz_type *type)
{
	if (parser->f->src[parser->f->pos] != ':')
		goto err_type_indicator_not_found;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	return parse_type(parser, type);
err_type_indicator_not_found:
	printf("amc: type_pair_parse_name: %lld,%lld: "
			"Type indicator not found\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int parse_type(struct parser *parser, yz_type *result)
{
	str token = TOKEN_NEW;
	yz_user_type *type = NULL;
	if (result == NULL)
		goto err_type_null;
	if (parser->f->src[parser->f->pos] == '*') {
		return parse_type_ptr(parser, result);
	} else if (parser->f->src[parser->f->pos] == '[') {
		return parse_type_array(parser, result);
	}
	if (token_read_before(SPECIAL_TOKEN_END, &token, parser->f) == NULL)
		return 1;
	if (parser->f->src[parser->f->pos] == '.') {
		if (type_get_from_module(&token, result, parser))
			return 1;
		return 0;
	}
	file_skip_space(parser->f);
	result->type = yz_type_get(&token);
	result->v = NULL;
	if (result->type != AMC_ERR_TYPE)
		return 0;
	if ((type = yz_user_type_find(&token, parser->scope)) == NULL)
		return 1;
	return parse_type_user(type, result);
err_type_null:
	printf("amc: parse_type: %lld,%lld: ARG is empty!\n",
			parser->f->cur_line, parser->f->cur_column);
	return 1;
}

int parse_type_name_pair(struct parser *parser, str *name, yz_type *type)
{
	str token = TOKEN_NEW;
	if (symbol_read_name(&token, parser->f))
		return 1;
	str_copy(&token, name);
	return type_pair_parse_type(parser, type);
}

int parse_type_user(yz_user_type *type, yz_type *result)
{
	switch (type->type) {
	case YZ_STRUCT:
		result->type = YZ_STRUCT;
		result->v = type->data.struct_;
		break;
	case YZ_ENUM:
		result->type = YZ_ENUM;
		result->v = type->data.enum_;
		break;
	default: goto err_unsupport; break;
	}
	return 0;
err_unsupport:
	printf("amc: parse_type_user: Unsupport type: '%s':%d\n",
			yz_get_raw_type_name(type->type),
			type->type);
	return 1;
}

yz_user_type *yz_user_type_find(str *s, struct scope *scope)
{
	yz_user_type *result = sctrie_find_elem(&scope->types, s->s, s->len);
	if (result != NULL)
		return result;
	if (scope->parent != NULL)
		return yz_user_type_find(s, scope->parent);
	return NULL;
}
