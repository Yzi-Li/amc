#ifndef AMC_SYMBOL_H
#define AMC_SYMBOL_H
#include "../utils/cint.h"
#include "../utils/str/str.h"
#include "backend/scope.h"
#include "backend/symbol.h"
#include "comptime/hook.h"
#include "comptime/symbol.h"
#include "file.h"
#include "type.h"
#include "struct.h"
#include <limits.h>

struct scope;

enum SYMG {
	SYMG_SYM  = 0,
	SYMG_FUNC = 1
};

#define SYM_GROUPS_SIZE 2

enum SYM_TYPE {
	SYM_FUNC,
	SYM_FUNC_ARG,
	SYM_IDENTIFIER,
	SYM_KEYWORD,
	SYM_STRUCT_ELEM
};

struct symbol_flag {
	unsigned int can_null:1,
	             only_declaration:1,
	             rec:1,
	             toplevel:1,
	             in_block:1,
	             is_init:1,
	             mut:1;
	struct comptime_symbol_flag comptime_flag;
};

struct symbol {
	char *name;
	u32 name_len;
	int (*parse_function)(struct file *f, struct symbol *sym,
			struct scope *scope);
	struct symbol_flag flags;
	enum SYM_TYPE type;

	u8 argc;
	yz_val result_type;
	struct symbol **args;

	backend_symbol_status *backend_status;
	struct hooks *hooks;
};

struct symbol_group {
	const char *name;
	u8 size;
	struct symbol **symbols;
};

enum SCOPE_STATUS_TYPE {
	SCOPE_AFTER_IF,
	SCOPE_IN_BLOCK,
	SCOPE_IN_LOOP,
	SCOPE_TOP
};

struct scope {
	struct symbol *fn;
	int indent;
	struct scope *parent;

	backend_scope_status *status;
	enum SCOPE_STATUS_TYPE status_type;

	struct {
		yz_struct **elems;
		int count;
	} structures;
	struct symbol_group sym_groups[SYM_GROUPS_SIZE];
};

int symbol_args_append(struct symbol *self, struct symbol *sym);
int symbol_check_name(const char *name, int len);
int symbol_find(str *token, struct symbol **result, struct scope *scope);
int symbol_find_in_group(str *token, struct symbol_group *group,
		struct symbol **result);
int symbol_find_in_group_in_scope(str *token, struct symbol **result,
		struct scope *scope, enum SYMG group_type);
int symbol_register(struct symbol *symbol, struct symbol_group *group);

void free_symbol(struct symbol *sym);
void free_symbol_group(struct symbol **syms, int count);

#endif
