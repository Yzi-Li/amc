#include "../include/scope.h"

void scope_free(struct scope *scope)
{
	symbol_group_free(&scope->sym_groups[SYMG_FUNC]);
	symbol_group_free(&scope->sym_groups[SYMG_SYM]);
}
