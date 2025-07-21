#include "include/token.h"
#include "../include/comptime/hook.h"
#include "../include/decorator/decorator.h"
#include "../include/token.h"
#include "../utils/converter.h"
#include <stdio.h>
#include <stdlib.h>

static int decorator_arg_get_val(str *token, yz_val *arg);
static int parse_decorator_arg(const char *se, struct file *f, void *data);
static int parse_decorator_args(struct file *f, struct hook_callee *callee);

int decorator_arg_get_val(str *token, yz_val *arg)
{
	if (CHR_IS_NUM(token->s[0])) {
		arg->type.type = YZ_I32;
		arg->l = 0;
		if (str2int(token, &arg->l))
			return 1;
	}
	return 0;
}

int parse_decorator_arg(const char *se, struct file *f, void *data)
{
	struct hook_callee *callee = data;
	str token = TOKEN_NEW;
	if (token_read_before(se, &token, f) == NULL)
		return 1;
	token_clean_tail_space(&token);
	callee->argc += 1;
	callee->args = realloc(callee->args, sizeof(*callee->args) * callee->argc);
	callee->args[callee->argc - 1] = malloc(sizeof(**callee->args));
	if (decorator_arg_get_val(&token, callee->args[callee->argc - 1]))
		return 1;;
	if (f->src[f->pos] == ']')
		return -1;
	file_pos_next(f);
	file_skip_space(f);
	return 0;
}

int parse_decorator_args(struct file *f, struct hook_callee *callee)
{
	if (f->src[f->pos] != '[')
		return 0;
	file_pos_next(f);
	file_skip_space(f);
	if (token_parse_list(",]", callee, f, parse_decorator_arg))
		return 1;
	return 0;
}

int parse_decorator(struct decorators *self, struct file *f)
{
	struct hook_callee *callee = NULL;
	struct decorator *dec = NULL;
	str token = TOKEN_NEW;
	if (f->src[f->pos] != '@')
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	if (token_read_before(SPECIAL_TOKEN_END, &token, f) == NULL)
		return 1;
	if ((dec = get_decorator(&token)) == NULL)
		return 1;
	callee = calloc(1, sizeof(*callee));
	callee->apply = dec->apply;
	if (parse_decorator_args(f, callee))
		return 1;
	if (hook_append(&self->hooks->times[dec->time], callee))
		return 1;
	self->has = 1;
	return 0;
}
