/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/decorator.h"
#include "../include/backend.h"
#include "../include/comptime/hook.h"
#include "../include/parser.h"
#include "../include/token.h"
#include "../utils/converter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int dec_c_fn(struct parser *parser, struct hook_callee *callee);
static int dec_syscall(struct parser *parser, struct hook_callee *callee);

static struct decorator decorators[] = {
	{ "c.fn", dec_c_fn, HOOK_FUNC_CALL_AFTER },
	{ "syscall", dec_syscall, HOOK_FUNC_CALL_AFTER }
};

static int decorator_arg_get_val(str *token, yz_val *arg);
static int parse_decorator_arg(const char *se, struct file *f, void *data);
static int parse_decorator_args(struct file *f, struct hook_callee *callee);

int dec_c_fn(struct parser *parser, struct hook_callee *callee)
{
	if (callee->argc != 1)
		goto err_arg_failed;
	if (backend_call(dec_c_fn)(&parser->sym->name, parser->sym->argc))
		goto err_backend_failed;
	return 0;
err_arg_failed:
	printf("amc: dec_c_fn: Argument failed\n");
	return 1;
err_backend_failed:
	printf("amc: dec_c_fn: Backend call failed!\n");
	return 1;
}

int dec_syscall(struct parser *parser, struct hook_callee *callee)
{
	if (callee->argc != 1)
		goto err_arg_failed;
	if (!YZ_IS_DIGIT(callee->args[0]->type.type))
		goto err_arg_failed;
	if (backend_call(dec_syscall)(callee->args[0]->i, parser->sym->argc))
		goto err_backend_failed;
	return 0;
err_arg_failed:
	printf("amc: dec_syscall: Argument failed\n");
	return 1;
err_backend_failed:
	printf("amc: dec_syscall: Backend call failed!\n");
	return 1;
}

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

struct decorator *get_decorator(str *token)
{
	if (token->s[0] == '@') {
		token->len -= 1;
		token->s = &token->s[1];
	}
	for (int i = 0; i < LENGTH(decorators); i++) {
		if (token->len != strlen(decorators[i].name))
			continue;
		if (strncmp(token->s, decorators[i].name, token->len) == 0)
			return &decorators[i];
	}
	return NULL;
}

int parse_decorator(struct decorators *self, struct file *f)
{
	struct hook_callee *callee = NULL;
	struct decorator *dec = NULL;
	char *err_msg;
	str token = TOKEN_NEW;
	if (f->src[f->pos] != '@')
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	if (token_read_before(" [\t\n", &token, f) == NULL)
		return 1;
	if ((dec = get_decorator(&token)) == NULL)
		goto err_dec_not_found;
	callee = calloc(1, sizeof(*callee));
	callee->apply = dec->apply;
	if (parse_decorator_args(f, callee))
		return 1;
	if (hook_append(&self->hooks->times[dec->time], callee))
		return 1;
	self->has = 1;
	return 0;
err_dec_not_found:
	err_msg = str2chr(token.s, token.len);
	printf("amc: parse_decorator: %lld,%lld: "ERROR_STR":\n"
			"| Decorator: '%s' not found!\n",
			f->cur_line, f->cur_column,
			err_msg);
	free(err_msg);
	return 1;
}

void free_decorators_noself(struct decorators *self)
{
	if (self == NULL)
		return;
	free_hooks_noself(self->hooks);
}
