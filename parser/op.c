#include "expr.h"
#include "op.h"
#include "../include/backend.h"
#include "../include/ptr.h"
#include "../utils/utils.h"

static int op_unary_extract_val(struct expr *e, struct scope *scope);
static int op_unary_get_addr(struct expr *e, struct scope *scope);

static int (*op_special_f[])() = {
	op_unary_extract_val,
	op_unary_get_addr
};

static int op_check_is_identifier(yz_val *v);
static int op_extract_val_check_type(yz_val *v);
static int op_get_addr_handle_val(struct expr *e);

static int op_assign(struct expr *e, struct scope *scope);
static int op_assign_check_vall(yz_val *val);
static int op_assign_check_vall_type(yz_val *val);

int op_unary_extract_val(struct expr *e, struct scope *scope)
{
	if (!EXPR_IS_UNARY(e))
		return 1;
	if (op_extract_val_check_type(e->valr))
		goto err_check_failed;
	if (backend_call(ops[e->op->id])(e))
		goto err_backend_failed;
	return 0;
err_check_failed:
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_backend_failed:
	printf("amc: op_unary_extract_val: Backend failed!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int op_unary_get_addr(struct expr *e, struct scope *scope)
{
	if (!EXPR_IS_UNARY(e))
		return 1;
	if (!op_check_is_identifier(e->valr))
		goto err_check_failed;
	if (op_get_addr_handle_val(e))
		goto err_check_failed;
	if (backend_call(ops[e->op->id])(e))
		goto err_backend_failed;
	return 0;
err_check_failed:
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_backend_failed:
	printf("amc: op_unary_get_addr: Backend failed!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int op_check_is_identifier(yz_val *v)
{
	struct symbol *sym = NULL;
	if (v->type != AMC_SYM)
		goto err_val_not_identifier;
	sym = v->v;
	if (sym->args != NULL)
		goto err_val_not_identifier;
	if (sym->args == NULL && sym->argc == 0)
		goto err_val_not_identifier;
	return 1;
err_val_not_identifier:
	printf("amc: op_check_is_identifier: Value is not identifier!\n");
	return 0;
}

int op_extract_val_check_type(yz_val *v)
{
	yz_val *type = NULL;
	if (!op_check_is_identifier(v))
		return 1;
	type = &((struct symbol*)v->v)->result_type;
	if (type->type != YZ_PTR)
		goto err_val_not_ptr;
	return 0;
err_val_not_ptr:
	printf("amc: op_extract_val_check_type: Value is not pointer!\n");
	return 1;
}

int op_get_addr_handle_val(struct expr *e)
{
	struct symbol *sym = e->valr->v;
	yz_ptr *ptr = malloc(sizeof(yz_ptr));
	expr_free_val(e->valr);
	e->valr = malloc(sizeof(*e->valr));
	e->valr->v = ptr;
	e->valr->type = YZ_PTR;
	e->sum_type = &e->valr->type;
	ptr->ref.type = AMC_SYM;
	ptr->ref.v = sym;
	return 0;
}

int op_assign(struct expr *e, struct scope *scope)
{
	char *name = NULL;
	struct symbol *sym = e->vall->v;
	if (op_assign_check_vall(e->vall))
		return 1;
	if (e->op->id == OP_ASSIGN) {
		name = str2chr(sym->name, sym->name_len);
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
	err_msg = str2chr(sym->name, sym->name_len);
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

int op_apply_special(struct expr *e, struct scope *scope)
{
	int func_id = 0;
	if (REGION_INT(e->op->id, OP_ASSIGN, OP_ASSIGN_SUB))
		return op_assign(e, scope);
	func_id = e->op->id - OP_SPECIAL_START - 5;
	if (func_id < LENGTH(op_special_f))
		return op_special_f[func_id](e, scope);
	return 0;
}
