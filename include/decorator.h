#ifndef AMC_DECORATOR_H
#define AMC_DECORATOR_H
#include "comptime/hook.h"

struct decorator {
	const char *name;
	hook_parser apply;
	enum HOOK_TIME time;
};

struct decorators {
	unsigned int has:1, used:1;
	struct hooks *hooks;
};

#endif
