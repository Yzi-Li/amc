#include "../include/backend.h"
#include "../include/decorator/decorator.h"
#include "../include/symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int dec_syscall(struct hook_callee *callee);

static struct decorator decorators[] = {
	{ "syscall", dec_syscall, HOOK_FUNC_CALL_AFTER }
};

int dec_syscall(struct hook_callee *callee)
{
	if (callee->argc != 1)
		goto err_arg_failed;
	if (!YZ_IS_DIGIT(callee->args[0]->type))
		goto err_arg_failed;
	if (backend_call(syscall)(callee->args[0]->i))
		goto err_backend_failed;
	return 0;
/*
err_defined:
	printf("amc: dec_syscall: Symbol: '%s' can "
			"only be declared but not defined.\n",
			sym->name);
	return 1;
*/
err_arg_failed:
	printf("amc: dec_syscall: Argument failed\n");
	return 1;
err_backend_failed:
	printf("amc: dec_syscall: Backend call failed!\n");
	return 1;
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
