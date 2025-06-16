#include "include/expr.h"
#include "include/identifier.h"
#include "include/op.h"
#include "../include/backend.h"
#include "../include/comptime/ptr.h"
#include "../include/ptr.h"
#include "../utils/utils.h"
#include <stdlib.h>

static int op_unary_extract_val(struct expr *e, struct scope *scope);
static int op_unary_get_addr(struct expr *e, struct scope *scope);

static int (*op_special_f[])(struct expr *e, struct scope *scope) = {
	op_unary_extract_val,
	op_unary_get_addr
};

static int op_check_is_identifier(yz_val *v);
static int op_extract_val_check_val(yz_val *v);
static int op_extract_val_handle_expr(struct expr *e);
static int op_get_addr_handle_val(struct expr *e);

static int op_assign_get_vall(struct expr *e, struct symbol **result);
static int op_assign_get_vall_expr(struct expr *e, struct symbol **result);
static int op_assign_get_vall_sym(struct symbol *sym, struct symbol **result);
static int op_cmp_ptr_and_null(struct expr *e);

int op_unary_extract_val(struct expr *e, struct scope *scope)
{
	if (!EXPR_IS_UNARY(e))
		return 1;
	if (e->op->sym == NULL)
		return 0;
	if (op_extract_val_check_val(e->valr))
		goto err_check_failed;
	if (backend_call(ops[e->op->id])(e))
		goto err_backend_failed;
	if (op_extract_val_handle_expr(e))
		return 1;
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
	if (sym->type != SYM_IDENTIFIER)
		goto err_val_not_identifier;
	return 1;
err_val_not_identifier:
	printf("amc: op_check_is_identifier: Value is not identifier!\n");
	return 0;
}

int op_extract_val_check_val(yz_val *v)
{
	struct symbol *sym = NULL;
	if (!op_check_is_identifier(v))
		return 1;
	sym = v->v;
	if (sym->result_type.type != YZ_PTR)
		goto err_val_not_ptr;
	if (!comptime_ptr_check_can_use(sym))
		return 1;
	return 0;
err_val_not_ptr:
	printf("amc: op_extract_val_check_type: Value is not pointer!\n");
	return 1;
}

int op_extract_val_handle_expr(struct expr *e)
{
	e->sum_type = &((yz_ptr*)
			((struct symbol*)e->valr->v)->result_type.v)
		->ref.type;
	return 0;
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

int op_assign_get_vall(struct expr *e, struct symbol **result)
{
	if (e->vall->type == AMC_SYM)
		return op_assign_get_vall_sym(e->vall->v, result);
	if (e->vall->type == AMC_EXPR)
		return op_assign_get_vall_expr(e->vall->v, result);
	return 1;
}

int op_assign_get_vall_expr(struct expr *e, struct symbol **result)
{
	if (e->op->id != OP_EXTRACT_VAL)
		return 1;
	if (e->valr->type != AMC_SYM)
		return 1;
	*result = e->valr->v;
	return 0;
}

int op_assign_get_vall_sym(struct symbol *sym, struct symbol **result)
{
	if (sym->args != NULL && sym->argc == 0)
		return 1;
	*result = sym;
	return 0;
}

int op_cmp_ptr_and_null(struct expr *e)
{
	struct symbol *sym = NULL;
	if (e->vall->type != AMC_SYM)
		return 1;
	sym = e->vall->v;
	if (sym->result_type.type != YZ_PTR)
		return 1;
	sym->flags.comptime_flag.checked_null = 1;
	return 0;
}

int op_apply_cmp(struct expr *e)
{
	if (e->valr->type == YZ_NULL)
		if (op_cmp_ptr_and_null(e))
			return 1;
	if (backend_call(ops[e->op->id])(e))
		goto err_backend_failed;
	return 0;
err_backend_failed:
	printf("amc: op_apply_cmp: Backend failed!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int op_apply_special(struct expr *e, struct scope *scope)
{
	int func_id = 0;
	func_id = e->op->id - OP_SPECIAL_START - 5;
	if (func_id < LENGTH(op_special_f))
		return op_special_f[func_id](e, scope);
	return 0;
}

int op_assign(struct file *f, struct expr *e, struct scope *scope)
{
	struct symbol *sym = NULL;
	if (op_assign_get_vall(e, &sym))
		return 1;
	return identifier_assign_val(f, sym, e->op->id, scope);
}
