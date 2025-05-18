// What fuck shit code?
#include "../include/expr.h"
#include "../include/ptr.h"

static int ptr_extract(yz_ptr *p, yz_val **ref);

int ptr_extract(yz_ptr *p, yz_val **ref)
{
	yz_ptr *cur = p;
	int level = 0;
	while (cur->ref.type == YZ_PTR) {
		cur = p->ref.v;
		level++;
	}
	if (ref != NULL)
		*ref = &cur->ref;
	if (cur->ref.type == AMC_SYM)
		*ref = &((struct symbol *)cur->ref.v)->result_type;
	return level;
}

int parse_ptr(str *token, yz_val *ptr)
{
	yz_ptr *box = malloc(sizeof(yz_ptr));
	ptr->type = YZ_PTR;
	ptr->v = box;
	return parse_type(token, &box->ref);
}

const char *yz_err_ptr_type(yz_val *val)
{
	printf("|v PTR: level: %d\n", ptr_extract(val->v, NULL));
	return "AMC_PTR";
}

yz_ptr *yz_ptr_get_from_val(yz_val *val)
{
	yz_val *tmp = NULL;
	if (val->type == AMC_EXPR)
		return yz_ptr_get_from_val(((struct expr*)val->v)->valr);
	if (val->type != AMC_SYM)
		return val->v;
	tmp = &((struct symbol*)val->v)->result_type;
	if (tmp->type != YZ_PTR)
		return NULL;
	return tmp->v;
}

int yz_ptr_is_equal(yz_ptr *l, yz_ptr *r)
{
	yz_val *lref, *rref;
	int llevel = 0, rlevel = 0;
	llevel = ptr_extract(l, &lref);
	rlevel = ptr_extract(r, &rref);
	if (llevel != rlevel)
		return 0;
	if (lref->type != rref->type)
		return 0;
	return 1;
}
