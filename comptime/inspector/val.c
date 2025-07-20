#include "../../include/comptime/ptr.h"
#include "../../include/comptime/val.h"

int comptime_check_sym_can_assign_val(struct symbol *sym, yz_val *val)
{
	if (sym->result_type.type == YZ_PTR && val->type.type == YZ_NULL) {
		if (!comptime_ptr_check_can_null(val, sym))
			return 0;
		return 1;
	}
	return 1;
}
