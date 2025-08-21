/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/backend.h"
#include "../include/scope.h"
#include <sctrie.h>
#include <stdio.h>

int scope_check_is_correct(struct scope *scope)
{
	if (scope->status == NULL)
		goto err_status_is_null;
	return 0;
err_status_is_null:
	printf("amc: scope_check_is_correct: Scope init failed!\n");
	return 1;
}

int scope_end(struct scope *scope)
{
	if (backend_call(scope_end)(scope->status))
		goto err_backend_failed;
	free_scope_noself(scope);
	return 0;
err_backend_failed:
	printf("amc: scope_end: Backend call failed!\n");
	return 1;
}

void free_scope(struct scope *scope)
{
	if (scope == NULL)
		return;
	free_scope_noself(scope);
	free(scope);
}

void free_scope_noself(struct scope *scope)
{
	if (scope == NULL)
		return;
	free_symbol_group(scope->sym_groups[SYMG_FUNC].symbols,
			scope->sym_groups[SYMG_FUNC].count);
	free_symbol_group(scope->sym_groups[SYMG_SYM].symbols,
			scope->sym_groups[SYMG_SYM].count);
	backend_call(scope_free)(scope->status);
	sctrie_free_tree_noself(&scope->types, free_yz_user_type);
}
