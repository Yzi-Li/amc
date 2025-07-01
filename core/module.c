#include "../include/module.h"
#include <stdio.h>
#include <string.h>

static const char *MODULE_NAME_VAILD_CHARS =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"_";

int module_append_child(yz_module *src, yz_module *dest)
{
	dest->children.count += 1;
	dest->children.modules = realloc(dest->children.modules,
			sizeof(*dest->children.modules) * dest->children.count);
	dest->children.modules[dest->children.count - 1] = src;
	return 0;
}

int check_module_name(str *name)
{
	for (int i = 0; i < name->len; i++) {
		if (strchr(MODULE_NAME_VAILD_CHARS, name->s[i]) == NULL)
			goto err_invalid_name;
	}
	return 1;
err_invalid_name: // TODO: better error message.
	printf("amc: test_module_name: Invaild character in module name!\n");
	return 0;
}

int check_module_parsed(str *name)
{
	return 0;
}
