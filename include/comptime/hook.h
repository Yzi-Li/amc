#ifndef AMC_COMPTIME_HOOK_H
#define AMC_COMPTIME_HOOK_H
#include "../type.h"

enum HOOK_TIME {
	HOOK_FUNC_CALL_AFTER,
	HOOK_FUNC_CALL_BEFORE,

	HOOK_TIME_COUNT
};

struct hook_callee;
typedef int (*hook_parser)(struct hook_callee *hook);
struct hook_callee {
	int argc;
	yz_val **args;
	hook_parser apply;
};

struct hook {
	int count;
	struct hook_callee **s;
};

struct hooks {
	struct hook times[HOOK_TIME_COUNT];
};

int hook_append(struct hook *hook, struct hook_callee *callee);
int hook_apply(struct hook *hook);

void free_hook(struct hook *hook);
void free_hook_noself(struct hook *hook);
void free_hook_callee(struct hook_callee *callee);
void free_hook_callee_args(int argc, yz_val **args);
void free_hooks(struct hooks *hooks);

#endif
