/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/backend.h"
#include "../include/comptime/hook.h"
#include "../include/scope.h"
#include "../include/symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int symbol_args_append(struct symbol *self, struct symbol *sym)
{
	self->argc += 1;
	self->args = realloc(self->args, sizeof(*self->args) * self->argc);
	self->args[self->argc - 1] = sym;
	return 0;
}

int symbol_check_name(const char *name, int len)
{
	char *c = NULL;
	if (CHR_IS_NUM(name[0]))
		goto err_num_in_name_begin;
	for (int i = 0; i < len; i++) {
		if ((c = strchr("!@#$%^&*()", name[i])) != NULL)
			goto err_c_in_name;
	}
	return 0;
err_num_in_name_begin:
	printf("amc: symbol_check_name: Number '%c' in symbol name begin!\n",
			name[0]);
	return 1;
err_c_in_name:
	printf("amc: symbol_check_name: '%c' in symbol name!\n", *c);
	return 1;
}

int symbol_find(str *token, struct symbol **result,
		struct scope *scope, enum SYMG group_type)
{
	if (symbol_find_in_group(token, &scope->sym_groups[group_type],
				result))
		return 1;
	if (scope->parent != NULL)
		return symbol_find(token, result, scope->parent, group_type);
	return 0;
}

int symbol_find_in_group(str *token, struct symbol_group *group,
		struct symbol **result)
{
	for (int i = 0; i < group->count; i++) {
		if (token->len != group->symbols[i]->name.len)
			continue;
		if (strncmp(token->s, group->symbols[i]->name.s,
					token->len) == 0) {
			*result = group->symbols[i];
			return 1;
		}
	}
	return 0;
}

struct symbol *symbol_pop(struct symbol_group *group)
{
	struct symbol *result;
	group->count -= 1;
	result = group->symbols[group->count];
	group->symbols[group->count] = NULL;
	return result;
}

int symbol_register(struct symbol *symbol, struct symbol_group *group)
{
	struct symbol *tmp = NULL;
	if (symbol_check_name(symbol->name.s, symbol->name.len))
		return 1;
	if (symbol_find_in_group(&symbol->name, group, &tmp)) {
		if (!tmp->flags.only_declaration)
			goto err_defined;
	}
	group->count += 1;
	if (group->count > group->size)
		group->size += 1;
	group->symbols = realloc(group->symbols,
			sizeof(struct symbol*) * group->size);
	group->symbols[group->count - 1] = symbol;
	return 0;
err_defined:
	printf("amc: symbol_register: symbol: \"%s\" defined!\n",
			symbol->name.s);
	return 1;
}

void free_symbol(struct symbol *sym)
{
	free_symbol_group(sym->args, sym->argc);
	backend_call(symbol_status_free)(sym->backend_status);
	free_yz_type_noself(&sym->result_type);
	free_hooks_noself(sym->hooks);
	str_free_noself(&sym->name);
	free_safe(sym);
}

void free_symbol_group(struct symbol **syms, int count)
{
	if (syms == NULL)
		return;
	for (int i = 0; i < count; i++) {
		if (syms[i] == NULL)
			continue;
		free_symbol(syms[i]);
	}
	free_safe(syms);
}
