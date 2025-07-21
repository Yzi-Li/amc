#include "include/keywords.h"
#include "../include/symbol.h"
#include "../utils/utils.h"
#include <string.h>

#define KW_DEF(NAME, NAME_LEN, PARSE_FUNC,            \
		REC, TOPLEVEL, IN_BLOCK)              \
	{                                             \
		.name           = NAME,               \
		.name_len       = NAME_LEN,           \
		.parse_function = PARSE_FUNC,         \
		.flags = {                            \
			.can_null         = 0,        \
			.only_declaration = 0,        \
			.rec              = REC,      \
			.toplevel         = TOPLEVEL, \
			.in_block         = IN_BLOCK, \
			.is_init          = 0,        \
			.mut              = 0         \
		}                                     \
	}

#define KW_REC 1
#define KW_TOPLEVEL 1
#define KW_IN_BLOCK 1

static struct symbol keywords[] = {
	KW_DEF("elif",   4, parse_elif,      KW_REC, !KW_TOPLEVEL,  KW_IN_BLOCK),
	KW_DEF("elif",   4, parse_elif,      KW_REC, !KW_TOPLEVEL,  KW_IN_BLOCK),
	KW_DEF("else",   4, parse_else,      KW_REC, !KW_TOPLEVEL,  KW_IN_BLOCK),
	KW_DEF("fn",     2, parse_func_def, !KW_REC,  KW_TOPLEVEL, !KW_IN_BLOCK),
	KW_DEF("if",     2, parse_if,        KW_REC, !KW_TOPLEVEL,  KW_IN_BLOCK),
	KW_DEF("let",    3, parse_let,      !KW_REC,  KW_TOPLEVEL,  KW_IN_BLOCK),
	KW_DEF("mod",    3, parse_mod,      !KW_REC,  KW_TOPLEVEL, !KW_IN_BLOCK),
	KW_DEF("pub",    3, parse_pub,      !KW_REC,  KW_TOPLEVEL, !KW_IN_BLOCK),
	KW_DEF("ret",    3, parse_func_ret, !KW_REC, !KW_TOPLEVEL,  KW_IN_BLOCK),
	KW_DEF("struct", 6, parse_struct,   !KW_REC,  KW_TOPLEVEL,  KW_IN_BLOCK),
	KW_DEF("while",  5, parse_while,     KW_REC, !KW_TOPLEVEL,  KW_IN_BLOCK),
};

int keyword_end(struct file *f)
{
	if (parse_comment(f))
		return 0;
	if (f->src[f->pos] == '\n')
		file_line_next(f);
	return 0;
}

int keyword_find(str *token, struct symbol **result)
{
	for (int i = 0; i < LENGTH(keywords); i++) {
		if (token->len != keywords[i].name_len)
			continue;
		if (strncmp(token->s, keywords[i].name, token->len) == 0) {
			*result = &keywords[i];
			return 1;
		}
	}
	return 0;
}
