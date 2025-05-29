#include "include/array.h"
#include "include/ptr.h"
#include "include/type.h"
#include "../include/token.h"

int parse_type(struct file *f, yz_val *type)
{
	char *err_msg = NULL;
	str token = TOKEN_NEW;
	if (type == NULL)
		goto err_type_null;
	if (f->src[f->pos] == '*') {
		return parse_type_ptr(f, type);
	} else if (f->src[f->pos] == '[') {
		return parse_type_array(f, type);
	}
	if (token_read_before(" \t\n:,)", &token, f) == NULL)
		return 1;
	file_skip_space(f);
	type->type = yz_type_get(&token);
	type->v = NULL;
	return 0;
err_type_null:
	printf("amc: parse_type: ARG is empty!\n");
	return 1;
}
