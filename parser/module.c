#include "include/keywords.h"
#include "include/utils.h"
#include "../include/module.h"
#include "../include/parser.h"
#include "../include/token.h"

static int module_read_name(struct file *f, str *result);

int module_read_name(struct file *f, str *result)
{
	i64 orig_column = f->cur_column,
	    orig_line = f->cur_line;
	while (f->src[f->pos] != '\0') {
	}
	if (!check_module_name(result))
		return err_print_pos(__func__, NULL,
				orig_line, orig_column);
	return 0;
}

int parse_import(struct parser *parser)
{
	return 0;
}

int parse_mod(struct parser *parser)
{
	str name = TOKEN_NEW;
	yz_module *result = NULL;
	if (module_read_name(parser->f, &name))
		return 1;
	result = calloc(1, sizeof(*result));
	result->name = str2chr(name.s, name.len);
	result->name_len = name.len;
	return 0;
}
