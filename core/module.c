#include "../include/module.h"
#include "../include/parser.h"
#include <stdio.h>
#include <string.h>

static const char *MODULE_NAME_VAILD_CHARS =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"_";

str *module_path2real(str *path)
{
	str *result = str_new();
	str_expand(result, global_parser.root_dir.len
			+ path->len
			+ 5);
	snprintf(result->s, result->len, "%s/%s.yz",
			global_parser.root_dir.s,
			path->s);
	return result;
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

void free_yz_module(yz_module *src)
{
	free(src->name.s);
	free(src->path.s);
	free_scope(src->scope);
	free(src);
}
