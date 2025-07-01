#include "include/keywords.h"
#include "include/module.h"
#include "include/utils.h"
#include "../include/module.h"
#include "../include/parser.h"
#include "../include/token.h"
#include <stdio.h>
#include <string.h>

static int module_read_name(struct file *f, str *result);

int module_read_name(struct file *f, str *result)
{
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	if (token_next(result, f))
		return 1;
	if (test_module_name(result))
		return err_print_pos(__func__, NULL,
				orig_line, orig_column);
	return 0;
}

int parse_import(struct file *f, struct symbol *sym, struct scope *scope)
{
	return 0;
}

int parse_mod(struct file *f, struct symbol *sym, struct scope *scope)
{
	str name = TOKEN_NEW;
	yz_module *result = NULL;
	if (module_read_name(f, &name))
		return 1;
	result = calloc(1, sizeof(*result));
	result->name = str2chr(name.s, name.len);
	result->name_len = name.len;
	return 0;
}

int test_module_name(str *name)
{
	for (int i = 0; i < name->len; i++) {
		if (strchr(MODULE_NAME_VAILD_CHARS, name->s[i]) == NULL)
			goto err_invalid_name;
	}
	return 0;
err_invalid_name: // TODO: better error message.
	printf("amc: test_module_name: Invaild character in module name!\n");
	return 1;
}
