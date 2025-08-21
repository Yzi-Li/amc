/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/comptime/hook.h"
#include <stdlib.h>
#include <string.h>

int hook_append(struct hook *hook, struct hook_callee *callee)
{
	if (hook == NULL)
		return 1;
	hook->count += 1;
	hook->s = realloc(hook->s, sizeof(*hook->s) * hook->count);
	hook->s[hook->count - 1] = callee;
	return 0;
}

int hook_apply(struct parser *parser, struct hook *hook)
{
	if (hook == NULL)
		return 1;
	for (int i = 0; i < hook->count; i++) {
		if (hook->s[i]->apply(parser, hook->s[i]))
			return 1;
	}
	return 0;
}

struct hooks *hooks_inherit(struct hooks **src)
{
	struct hooks *result = NULL;
	if (src == NULL)
		return NULL;
	result = *src;
	*src = NULL;
	return result;
}

void free_hook(struct hook *hook)
{
	if (hook == NULL)
		return;
	free_hook_noself(hook);
	free(hook);
}

void free_hook_noself(struct hook *hook)
{
	if (hook == NULL)
		return;
	for (int i = 0; i < hook->count; i++)
		free_hook_callee(hook->s[i]);
	free(hook->s);
}

void free_hook_callee(struct hook_callee *callee)
{
	if (callee == NULL)
		return;
	free_hook_callee_args(callee->argc, callee->args);
	free(callee);
}

void free_hook_callee_args(int argc, yz_val **args)
{
	if (args == NULL)
		return;
	for (int i = 0; i < argc; i++) {
		if (args[i] == NULL)
			continue;
		free_yz_val(args[i]);
	}
	free(args);
}

void free_hooks(struct hooks *hooks)
{
	if (hooks == NULL)
		return;
	free_hooks_noself(hooks);
	free(hooks);
}

void free_hooks_noself(struct hooks *hooks)
{
	if (hooks == NULL)
		return;
	for (int i = 0; i < HOOK_TIME_COUNT; i++) {
		free_hook_noself(&hooks->times[i]);
	}
}
