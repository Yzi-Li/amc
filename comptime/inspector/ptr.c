#include "../../include/comptime/ptr.h"
#include <stdio.h>

static int check_val_type(yz_val *val);

int check_val_type(yz_val *val)
{
	struct symbol *sym = NULL;
	if (val->type == YZ_NULL)
		return 1;
	if (val->type != AMC_SYM)
		return 0;
	sym = val->v;
	if (sym->result_type.type == YZ_NULL)
		return 1;
	if (sym->result_type.type != YZ_PTR)
		goto err_not_ptr;
	if (sym->flags.can_null)
		return 1;
	return 0;
err_not_ptr:
	printf("amc[comptime:%s]: check_val_type:\n"
			"| Value(symbol): '%s' isn't pointer.\n",
			__FILE__, sym->name);
	return 0;
}

int comptime_ptr_check_can_null(yz_val *val, struct symbol *sym)
{
	if (!check_val_type(val))
		goto err;
	if (!sym->flags.can_null && !((struct symbol*)val->v)
			->flags.comptime_flag.checked_null)
		goto err_cannot_null;
	return 1;
err:
	printf("| comptime_ptr_check_can_null: "
			"Cannot use null for Symbol: '%s'\n",
			sym->name);
	return 0;
err_cannot_null:
	printf("amc: comptime_ptr_check_can_null:\n"
			"| ERROR: Cannot use null for symbol: '%s'!\n"
			"| HINT:  Use 'if' to check symbol before use it.\n",
			sym->name);
	return 0;
}

int comptime_ptr_check_can_ret(struct symbol *sym, struct symbol *fn)
{
	if (!sym->flags.can_null)
		return 1;
	if (fn->flags.can_null)
		return 1;
	if (sym->flags.comptime_flag.checked_null)
		return 1;
	printf("amc: comptime_ptr_check_can_ret:\n"
			"| Pointer: '%s' must be checked is null.\n"
			"| Function: '%s' must return a non null pointer.\n",
			sym->name, fn->name);
	return 0;
}

int comptime_ptr_check_can_use(struct symbol *sym)
{
	if (!sym->flags.can_null)
		return 1;
	if (sym->flags.comptime_flag.checked_null)
		return 1;
	printf("amc: comptime_ptr_check_can_use:\n"
			"| Pointer: '%s' must be checked is null.\n",
			sym->name);
	return 0;
}
