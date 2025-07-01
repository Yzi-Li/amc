#include "../include/comptime/hook.h"
#include <stdlib.h>

int hook_append(struct hook *hook, struct hook_callee *callee)
{
	hook->count += 1;
	hook->s = realloc(hook->s, sizeof(*hook->s) * hook->count);
	hook->s[hook->count - 1] = callee;
	return 0;
}

int hook_apply(struct hook *hook)
{
	for (int i = 0; i < hook->count; i++) {
		if (hook->s[i]->apply(hook->s[i]))
			return 1;
	}
	return 0;
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
	for (int i = 0; i < hook->count; i++) {
		free_hook_callee(hook->s[i]);
	}
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
	for (int i = 0; i < HOOK_TIME_COUNT; i++) {
		free_hook(&hooks->times[i]);
	}
	free(hooks);
}
