#include "../include/backend.h"
#include "../include/scope.h"

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
	symbol_group_free(&scope->sym_groups[SYMG_FUNC]);
	symbol_group_free(&scope->sym_groups[SYMG_SYM]);
	if (backend_call(scope_end)(scope->status))
		goto err_backend_failed;
	return 0;
err_backend_failed:
	printf("amc: scope_end: Backend call failed!\n");
	return 1;
}
