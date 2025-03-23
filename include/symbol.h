#ifndef AMC_SYMBOL_H
#define AMC_SYMBOL_H
#include "../utils/cint.h"
#include "../utils/str/str.h"
#include "file.h"
#include "type.h"
#include <limits.h>

struct scope;

enum SYMG {
	SYMG_SYM  = 0,
	SYMG_FUNC = 1
};

#define SYM_GROUPS_SIZE 2

struct symbol_flag {
	unsigned int toplevel:1, in_block:1, mut:1;
};

struct symbol {
	const char *name;
	u32 name_len;
	int (*parse_function)(struct file *f, struct symbol *sym,
			struct scope *scope);
	struct symbol_flag flags;

/**
 * if args == NULL and argc == 1 =>
 *   symbol is identifier.
 */
	u8 argc;
	enum YZ_TYPE result_type;
	enum YZ_TYPE *args;
};

struct symbol_group {
	const char *name;
	u8 size;
	struct symbol **symbols;
};

struct scope {
	struct symbol *fn;
	struct scope *parent;
	struct symbol_group sym_groups[SYM_GROUPS_SIZE];
};

int symbol_args_append(struct symbol *self, enum YZ_TYPE type);
int symbol_find(str *token, struct symbol **result, struct scope *scope);
int symbol_find_in_group(str *token, struct symbol_group *group,
		struct symbol **result);
int symbol_find_in_group_in_scope(str *token, struct symbol **result,
		struct scope *scope, enum SYMG group_type);
void symbol_group_free(struct symbol_group *group);
int symbol_register(struct symbol *symbol, struct symbol_group *group);

#endif
