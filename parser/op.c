#include "op.h"
#include "../include/backend.h"
#include "../include/token.h"
#include "../utils/utils.h"

static int op_assign(struct expr *e, struct scope *scope);
static int op_assign_check_vall(yz_val *val);
static int op_assign_check_vall_type(yz_val *val);

int op_assign_check_vall(yz_val *val)
{
	char *err_msg = NULL;
	struct symbol *sym = val->v;
	if (op_assign_check_vall_type(val))
		return 1;
	if (!sym->flags.mut)
		goto err_sym_is_immut;
	return 0;
err_sym_is_immut:
	err_msg = tok2str(sym->name, sym->name_len);
	printf("amc: op_assign_check_vall: Symbol: \"%s\" is immutable!\n",
			err_msg);
	free(err_msg);
	return 1;
}

int op_assign_check_vall_type(yz_val *val)
{
	struct symbol *sym = val->v;
	if (val->type != AMC_SYM)
		return 1;
	if (sym->args != NULL)
		return 1;
	if (sym->argc == 0)
		return 1;
	return 0;
}

int op_assign(struct expr *e, struct scope *scope)
{
	char *name = NULL;
	struct symbol *sym = e->vall->v;
	if (op_assign_check_vall(e->vall))
		return 1;
	if (e->op->id == OP_ASSIGN) {
		name = tok2str(sym->name, sym->name_len);
		if (backend_call(var_set)(name, e->valr))
			goto err_backend_failed;
		free(name);
		return 0;
	}
	if (backend_call(ops[e->op->id])(e))
		goto err_backend_failed;
	return 0;
err_backend_failed:
	printf("amc: op_assign: Backend call failed!\n");
	return 1;
}

int op_apply_special(struct expr *e, struct scope *scope)
{
	if (REGION_INT(e->op->id, OP_ASSIGN, OP_ASSIGN_SUB))
		return op_assign(e, scope);
	return 0;
}
