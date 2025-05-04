#include "include/asf.h"
#include "../../include/symbol.h"
#include "../../include/token.h"

int asf_op_assign(struct expr *e)
{
	char *name = NULL;
	struct symbol *sym = e->vall->v;
	name = tok2str(sym->name, sym->name_len);
	return asf_var_set(name, e->valr);
}

int asf_op_assign_add(struct expr *e)
{
	return 0;
}

int asf_op_assign_div(struct expr *e)
{
	return 0;
}

int asf_op_assign_mul(struct expr *e)
{
	return 0;
}

int asf_op_assign_sub(struct expr *e)
{
	return 0;
}
