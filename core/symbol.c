#include "../include/symbol.h"
#include <stdlib.h>

int symbol_args_append(struct symbol *self, enum YZ_TYPE type)
{
	self->argc += 1;
	self->args = realloc(self->args, sizeof(enum YZ_TYPE) * self->argc);
	self->args[self->argc - 1] = type;

	return 0;
}

int symbol_find(str *token, struct symbol **result, struct scope *scope)
{
	for (int i = 0; i < SYM_GROUPS_SIZE; i++) {
		if (symbol_find_in_group(token, &scope->sym_groups[i], result))
			return 1;
	}
	if (scope->parent != NULL)
		return symbol_find(token, result, scope->parent);
	return 0;
}

int symbol_find_in_group(str *token, struct symbol_group *group,
		struct symbol **result)
{
	for (int i = 0; i < group->size; i++) {
		if (token->len != group->symbols[i]->name_len)
			continue;
		if (strncmp(token->s, group->symbols[i]->name,
					token->len) == 0) {
			*result = group->symbols[i];
			return 1;
		}
	}

	return 0;
}

int symbol_find_in_group_in_scope(str *token, struct symbol **result,
		struct scope *scope, enum SYMG group_type)
{
	if (symbol_find_in_group(token, &scope->sym_groups[group_type],
				result))
		return 1;
	if (scope->parent != NULL)
		return symbol_find_in_group_in_scope(token, result,
				scope->parent, group_type);
	return 0;
}

void symbol_group_free(struct symbol_group *group)
{
	for (int i = 0; i < group->size; i++)
		free(group->symbols[i]);
	free(group->symbols);
}

int symbol_register(struct symbol *symbol, struct symbol_group *group)
{
	group->size += 1;
	group->symbols = realloc(group->symbols,
			sizeof(struct symbol*) * group->size);
	group->symbols[group->size - 1] = symbol;
	return 0;
}
