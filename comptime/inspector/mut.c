#include "../../include/comptime/mut.h"
#include <stdio.h>

int comptime_check_struct_elem_can_assign(struct symbol *sym,
		struct symbol *elem)
{
	if (!elem->flags.mut)
		goto err_elem_is_immut;
	if (!sym->flags.mut)
		goto err_parent_is_immut;
	return 1;
err_elem_is_immut:
	printf("amc: comptime_check_struct_elem_can_assign: "
			"ERROR: Element '%s' is immutable!\n",
			elem->name);
	return 0;
err_parent_is_immut:
	printf("amc: comptime_check_struct_elem_can_assign: "
			"ERROR: symbol is immutable!\n"
			"| HINT:  Element: '%s' is mutable.\n"
			"|        Struct: '%s' is immutable!\n",
			elem->name, sym->name);
	return 0;
}

int comptime_check_sym_can_assign(struct symbol *sym)
{
	if (!sym->flags.mut && sym->flags.is_init)
		goto err_sym_is_immut;
	return 1;
err_sym_is_immut:
	printf("amc: comptime_check_sym_can_assign: "
			"ERROR: Symbol: \"%s\" is immutable!\n", sym->name);
	return 0;
}
