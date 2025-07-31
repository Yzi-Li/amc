#include "../../include/comptime/struct.h"
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
			elem->name.s);
	return 0;
err_parent_is_immut:
	printf("amc: comptime_check_struct_elem_can_assign: "
			"ERROR: symbol is immutable!\n"
			"| HINT:  Element: '%s' is mutable.\n"
			"|        Struct: '%s' is immutable!\n",
			elem->name.s, sym->name.s);
	return 0;
}
