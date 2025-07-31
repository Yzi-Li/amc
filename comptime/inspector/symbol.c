#include "../../include/comptime/ptr.h"
#include "../../include/comptime/symbol.h"
#include <stdio.h>

int comptime_check_sym_can_assign(struct symbol *sym)
{
	if (!sym->flags.mut && sym->flags.is_init)
		goto err_sym_is_immut;
	return 1;
err_sym_is_immut:
	printf("amc: comptime_check_sym_can_assign: "
			"ERROR: Symbol: \"%s\" is immutable!\n", sym->name.s);
	return 0;
}

int comptime_check_sym_can_assign_val(struct symbol *sym, yz_val *val)
{
	if (sym->result_type.type == YZ_PTR && val->type.type == YZ_NULL) {
		if (!comptime_ptr_check_can_null(val, sym))
			return 0;
		return 1;
	}
	return 1;
}
