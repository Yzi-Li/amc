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
