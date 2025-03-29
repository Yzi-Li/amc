#include "keywords.h"
#include "../include/symbol.h"
#include "../utils/utils.h"

static struct symbol keywords[] = {
	//{"const",  5, parse_const,    {0, 1, 0}},
	//{"elif",   4, parse_elif,     {0, 1, 0}},
	//{"else",   4, parse_else,     {0, 1, 0}},
	{"fn",     2, parse_func_def, {1, 0, 0}},
	//{"for",    3, parse_for,      {0, 1, 0}},
	//{"from",   4, parse_from,     {0, 1, 0}},
	//{"if",     2, parse_if,       {0, 1, 0}},
	//{"import", 6, parse_import,   {1, 0, 0}},
	{"let",    3, parse_let,      {1, 1, 0}},
	//{"module", 6, parse_module,   {1, 0, 0}},
	//{"mut",    3, parse_mut,      {0, 1, 0}},
	//{"ref",    3, parse_ref,      {0, 1, 0}},
	{"ret",    3, parse_func_ret, {0, 1, 0}},
	//{"struct", 6, parse_struct,   {1, 1, 0}},
	//{"while",  5, parse_while,    {0, 1, 0}},
};

int keyword_find(str *token, struct symbol **result)
{
	for (int i = 0; i < LENGTH(keywords); i++) {
		if (strncmp(token->s, keywords[i].name, token->len) == 0) {
			*result = &keywords[i];
			return 1;
		}
	}
	return 0;
}
