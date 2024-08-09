#include "../include/symbol.h"
#include <stdlib.h>

static struct symbol_groups {
	int len;
	struct symbol_group **groups;
} *symbol_groups = NULL;

int symbol_args_append(struct symbol *self, enum YZ_TYPE type)
{
	self->argc += 1;
	self->args = realloc(self->args, sizeof(enum YZ_TYPE) * self->argc);
	self->args[self->argc - 1] = type;

	return 0;
}

int symbol_find(str *token, struct symbol **result)
{
	for (int i = 0; i < symbol_groups->len; i++) {
		if (symbol_find_in_group(token, i, result))
			return 1;
	}

	return 0;
}

int symbol_find_in_group(str *token, int group, struct symbol **result)
{
	for (int i = 0; i < symbol_groups->groups[group]->size; i++) {
		if (strncmp(token->s, symbol_groups->groups[group]->symbols[i]->name,
					token->len)
				== 0) {
			*result = symbol_groups->groups[group]->symbols[i];
			return 1;
		}
	}

	return 0;
}

int symbol_group_find(const char *name)
{
	for (int i = 0; i < symbol_groups->len; i++) {
		if (strcmp(name, symbol_groups->groups[i]->name) == 0) {
			return i;
		}
	}

	return -1;
}

int symbol_group_register(struct symbol_group *group)
{
	symbol_groups->len += 1;
	symbol_groups->groups = realloc(
			symbol_groups->groups,
			sizeof(struct symbol_group) * symbol_groups->len);
	if (symbol_groups == NULL)
		return 1;
	symbol_groups->groups[symbol_groups->len - 1] = group;
	return 0;
}

int symbol_register(struct symbol *symbol, int group)
{
	symbol_groups->groups[group]->size += 1;
	symbol_groups->groups[group]->symbols =
		realloc(symbol_groups->groups[group]->symbols,
				sizeof(struct symbol*) * symbol_groups->groups[group]->size);
	symbol_groups->groups[group]->symbols[symbol_groups->groups[group]->size - 1] = symbol;
	return 0;
}

int symbols_init()
{
	symbol_groups = malloc(sizeof(*symbol_groups));
	if (symbol_groups == NULL)
		return 1;
	symbol_groups->groups = NULL;
	symbol_groups->len = 0;
	struct symbol_group *sym = calloc(1, sizeof(struct symbol_group));
	struct symbol_group *func = calloc(1, sizeof(struct symbol_group));
	sym->name = "sym";
	sym->size = 0;
	sym->symbols = NULL;
	func->name = "func";
	func->size = 0;
	func->symbols = NULL;
	symbol_group_register(sym);
	symbol_group_register(func);
	return 0;
}
