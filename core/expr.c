#include "../include/expr.h"

void free_expr(struct expr *e)
{
	if (e == NULL)
		return;
	free_yz_val(e->vall);
	free_yz_val(e->valr);
	free_safe(e->op);
	free(e);
}
