#ifndef AMC_DECORATOR_H
#define AMC_DECORATOR_H
#include "../comptime/hook.h"
#include "../../utils/str/str.h"

struct decorator {
	const char *name;
	hook_parser apply;
	enum HOOK_TIME time;
};

struct decorator *get_decorator(str *token);

#endif
