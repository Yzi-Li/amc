#include "../include/type.h"
#include "../utils/utils.h"
#include "../utils/die.h"
#include <limits.h>

enum YZ_TYPE yz_get_int_size(long long l)
{
	if (l <= CHAR_MAX)
		return YZ_I8;
	if (l <= SHRT_MAX)
		return YZ_I16;
	if (l <= INT_MAX)
		return YZ_I32;
	if (l <= LLONG_MAX)
		return YZ_I64;
	return AMC_ERR_TYPE;
}

const char *yz_get_type_name(enum YZ_TYPE type)
{
	int index = type - YZ_TYPE_OFFSET;
	if (type < YZ_TYPE_OFFSET || index > LENGTH(yz_type_table)) {
		switch (type) {
		case AMC_SYM:
			return "AMC_SYM";
			break;
		case AMC_EXPR:
			return "AMC_EXPR";
			break;
		default:
			return "(Cannot get type)";
		}
		return NULL;
	}
	return yz_type_table[index].name;
}

enum YZ_TYPE yz_type_get(str *s)
{
	for (int i = 0, len = LENGTH(yz_type_table); i < len; i++) {
		if (strncmp(s->s, yz_type_table[i].name, s->len) == 0)
			return yz_type_table[i].type;
	}

	return AMC_ERR_TYPE;
}

int parse_type(str *token, enum YZ_TYPE *type)
{
	if (token->s[0] == '*')
		die("amc: unsupport type: pointer.");

	for (int i = 0; i < token->len; i++) {
		switch (token->s[i]) {
		case '(':
		case '[':
			return i;
			break;
		}
	}

	*type = yz_type_get(token);
	return 0;
}
