/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_SYMBOL_H
#define AMC_SYMBOL_H
#include "backend/symbol.h"
#include "type.h"
#include "../utils/cint.h"
#include "../utils/str/str.h"
#include <ctype.h>
#include <limits.h>

#define is_sym_chr(c) (isalpha(c) || isdigit(c) || (c) == '_' || (c) == '-')

enum SYMG {
	SYMG_SYM  = 0,
	SYMG_FUNC = 1,

	SYM_GROUPS_SIZE
};

enum SYM_TYPE {
	SYM_FUNC,
	SYM_FUNC_ARG,
	SYM_IDENTIFIER,
	SYM_KEYWORD,
	SYM_STRUCT_ELEM
};

struct symbol_flag {
	unsigned int can_null:1,
	             checked_null:1,
	             only_declaration:1,
	             rec:1,
	             toplevel:1,
	             in_block:1,
	             is_init:1,
	             mut:1;
};

struct parser;
struct scope;
struct symbol {
	str name, path;
	int (*parse_function)(struct parser *parser);
	struct symbol_flag flags;
	enum SYM_TYPE type;

	u8 argc;
	yz_type result_type;
	struct symbol **args;

	backend_symbol_status *backend_status;
	struct hooks *hooks;
};

struct symbol_group {
	const char *name;
	u8 size;
	struct symbol **symbols;
};

int symbol_args_append(struct symbol *self, struct symbol *sym);
int symbol_check_name(const char *name, int len);
int symbol_find(str *token, struct symbol **result,
		struct scope *scope, enum SYMG group_type);
int symbol_find_in_group(str *token, struct symbol_group *group,
		struct symbol **result);
int symbol_register(struct symbol *symbol, struct symbol_group *group);

void free_symbol(struct symbol *sym);
void free_symbol_group(struct symbol **syms, int count);

#endif
