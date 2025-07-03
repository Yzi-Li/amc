#ifndef AMC_SCOPE_H
#define AMC_SCOPE_H
#include "symbol.h"
#include "backend/scope.h"

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

int scope_check_is_correct(struct scope *scope);
int scope_end(struct scope *scope);

void free_scope(struct scope *scope);

#endif
