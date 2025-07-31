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
	free_scope(scope);
	if (backend_call(scope_end)(scope->status))
		goto err_backend_failed;
	return 0;
err_backend_failed:
	printf("amc: scope_end: Backend call failed!\n");
	return 1;
}

void free_scope(struct scope *scope)
{
	if (scope == NULL)
		return;
	free_symbol_group(scope->sym_groups[SYMG_FUNC].symbols,
			scope->sym_groups[SYMG_FUNC].size);
	free_symbol_group(scope->sym_groups[SYMG_SYM].symbols,
			scope->sym_groups[SYMG_SYM].size);
	sctrie_free_tree_noself(&scope->types, free_yz_user_type);
}
