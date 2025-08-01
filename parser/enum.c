#include "include/block.h"
#include "include/enum.h"
#include "include/keywords.h"
#include "include/type.h"
#include "include/symbol.h"
#include "include/utils.h"
#include "../include/backend.h"
#include "../include/parser.h"
#include "../include/token.h"
#include <sctrie.h>
#include <stdio.h>
#include <string.h>

static int enum_def_multi_line(yz_enum *self, struct parser *parser);
static int enum_def_multi_line_check_end(struct file *f);
static int enum_def_read_item(yz_enum *self, struct file *f);
static int enum_def_reg(yz_enum *self, struct scope *scope);
static int enum_def_reg_item(yz_enum *self, yz_enum_item *item);
static int enum_def_single_line(yz_enum *self, struct parser *parser);

int enum_def_multi_line(yz_enum *self, struct parser *parser)
{
	int ret = 0;
	while ((ret = enum_def_multi_line_check_end(parser->f)) != -1) {
		if (ret > 0)
			return 1;
		if (enum_def_single_line(self, parser))
			return 1;
	}
	return 0;
}

int enum_def_multi_line_check_end(struct file *f)
{
	if (f->src[f->pos] != '|')
		return -1;
	file_pos_next(f);
	file_skip_space(f);
	if (try_next_line(f))
		return 1;
	return 0;
}

int enum_def_read_item(yz_enum *self, struct file *f)
{
	yz_enum_item *item = calloc(1, sizeof(*item));
	str token = TOKEN_NEW;
	if (symbol_read(&token, f))
		return 1;
	str_copy(&token, &item->name);
	item->u = self->count;
	if (enum_def_reg_item(self, item))
		return 1;
	if (!try_next_line(f))
		return 1;
	return 0;
}

int enum_def_reg(yz_enum *self, struct scope *scope)
{
	yz_user_type *type = sctrie_append_elem(&scope->types, sizeof(*type),
			self->name.s, self->name.len);
	if (type == NULL)
		goto err_defined;
	type->type = YZ_ENUM;
	type->enum_ = self;
	return 0;
err_defined:
	printf("amc: enum_def_reg: "
			"Type defined: '%s'\n", self->name.s);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int enum_def_reg_item(yz_enum *self, yz_enum_item *item)
{
	self->count += 1;
	self->elems = realloc(self->elems, sizeof(*self->elems) * self->count);
	self->elems[self->count - 1] = item;
	return 0;
}

int enum_def_single_line(yz_enum *self, struct parser *parser)
{
	int ret = 0;
	while ((ret = enum_def_read_item(self, parser->f)) != -1) {
		if (ret > 0)
			return 1;
	}
	return 0;
}

yz_enum *yz_enum_find(str *s, struct scope *scope)
{
	struct yz_user_type *type = yz_user_type_find(s, scope);
	if (!type || type->type != YZ_ENUM)
		return NULL;
	return type->enum_;
}

yz_enum_item *yz_enum_item_find(str *s, yz_enum *src)
{
	for (int i = 0; i < src->count; i++) {
		if (s->len != src->elems[i]->name.len)
			continue;
		if (strncmp(s->s, src->elems[i]->name.s, s->len) == 0)
			return src->elems[i];
	}
	return NULL;
}

int parse_enum(struct parser *parser)
{
	str name = TOKEN_NEW;
	int ret = 0;
	yz_enum *self = calloc(1, sizeof(*self));
	if (parse_type_name_pair(parser, &name, &self->type))
		goto err_free_self;
	if (!block_check_start(parser->f))
		goto err_block_not_start;
	str_copy(&name, &self->name);
	if (!YZ_IS_DIGIT(self->type.type))
		goto err_not_digit;
	if (enum_def_reg(self, parser->scope))
		goto err_free_self;
	if (try_next_line(parser->f)) {
		if ((ret = enum_def_multi_line(self, parser)) > 0)
			return 1;
		if (ret == -1)
			return 1;
		return 0;
	}
	return enum_def_single_line(self, parser);
err_not_digit:
	printf("amc: parse_enum: %lld,%lld: "ERROR_STR": "
			"Enum can only be based on digit: '%s'\n"
			"| "HINT_STR": use 'i8' to 'i64', "
			"'u8' to 'u64' or 'char'.\n",
			parser->f->cur_line, parser->f->cur_column,
			self->name.s);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_block_not_start:
	printf("amc: parse_enum: %lld,%lld: Block not start!\n",
			parser->f->cur_line, parser->f->cur_column);
err_free_self:
	free_yz_enum(self);
	return 1;
}
