#ifndef AMC_SYMBOL_H
#define AMC_SYMBOL_H
#include "../utils/cint.h"
#include "../utils/str/str.h"
#include "file.h"
#include "type.h"

enum SYMG {
	SYMG_SYM  = 0,
	SYMG_FUNC = 1,
	SYMG_KW   = 2
};

struct symbol_flag {
	unsigned int toplevel:1, in_block:1, mut:1;
};

struct symbol {
	const char *name;
	u32 name_len;
	int (*parse_function)(struct file *f, struct symbol *sym, struct symbol *fn);
	struct symbol_flag flags;

	u8 argc;
	enum YZ_TYPE result_type;
	enum YZ_TYPE *args;
};

struct symbol_group {
	const char *name;
	u8 size;
	struct symbol **symbols;
};

int symbol_args_append(struct symbol *self, enum YZ_TYPE type);
int symbol_find(str *token, struct symbol **result);
int symbol_find_in_group(str *token, int group, struct symbol **result);
int symbol_group_find(const char *name);
int symbol_group_register(struct symbol_group *group);
int symbol_register(struct symbol *symbol, int group);
int symbols_init();

#endif
