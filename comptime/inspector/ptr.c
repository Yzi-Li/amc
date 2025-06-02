#include "../../include/comptime/ptr.h"

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
		return 0;
	if (sym->flags.can_null)
		return 1;
	return 0;
}

int comptime_ptr_check_can_null(yz_val *val, struct symbol *sym)
{
	char *err_msg;
	if (!check_val_type(val))
		goto err;
	if (!sym->flags.can_null && !((struct symbol *)val->v)
			->flags.comptime_flag.checked_null)
		goto err;
	return 1;
err:
	err_msg = str2chr(sym->name, sym->name_len);
	printf("amc: comptime_ptr_check_can_null:\n"
			"| ERROR: Cannot use null for symbol: '%s'!\n"
			"| NOTE:  Use 'if' to check symbol before use it.\n",
			err_msg);
	free(err_msg);
	return 0;
}

int comptime_ptr_check_can_ret(struct symbol *sym, struct symbol *fn)
{
	char *fn_name, *ptr_name;
	if (!sym->flags.can_null)
		return 1;
	if (fn->flags.can_null)
		return 1;
	if (sym->flags.comptime_flag.checked_null)
		return 1;
	fn_name = str2chr(fn->name, fn->name_len);
	ptr_name = str2chr(sym->name, sym->name_len);
	printf("amc: comptime_ptr_check_can_ret:\n"
			"| Pointer: '%s' must be checked is null.\n"
			"| Function: '%s' must return a non null pointer.\n",
			ptr_name, fn_name);
	free(fn_name);
	free(ptr_name);
	return 0;
}

int comptime_ptr_check_can_use(struct symbol *sym)
{
	char *err_msg;
	if (!sym->flags.can_null)
		return 1;
	if (sym->flags.comptime_flag.checked_null)
		return 1;
	err_msg = str2chr(sym->name, sym->name_len);
	printf("amc: comptime_ptr_check_can_use:\n"
			"| Pointer: '%s' must be checked is null.\n",
			err_msg);
	free(err_msg);
	return 0;
}
