// What fuck shit code?
#include "../include/expr.h"
#include "../include/ptr.h"
#include "../include/symbol.h"
#include "../include/type.h"
#include <stdio.h>

static int ptr_extract(yz_ptr *p, yz_val **ref);

int ptr_extract(yz_ptr *p, yz_val **ref)
{
	yz_ptr *cur = p;
	int level = 0;
	if (cur->ref.type == AMC_EXTRACT_VAL
			&& ((cur = yz_ptr_get_from_val_extracted(
						cur->ref.v,
						ref)) == NULL)) {
		return 0;
	}
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

yz_ptr *yz_ptr_get_from_val(yz_val *val)
{
	yz_val *tmp = NULL;
	if (val->type == AMC_EXPR)
		return yz_ptr_get_from_val(((struct expr*)val->v)->valr);
	if (val->type == AMC_EXTRACT_VAL)
		return yz_ptr_get_from_val_extracted(val->v, NULL);
	if (val->type != AMC_SYM)
		return val->v;
	tmp = &((struct symbol*)val->v)->result_type;
	if (tmp->type != YZ_PTR)
		return NULL;
	return tmp->v;
}

yz_ptr *yz_ptr_get_from_val_extracted(yz_extract_val *val, yz_val **type)
{
	yz_val *src = &val->elem->result_type;
	if (type != NULL)
		*type = src;
	if (src->type != YZ_PTR)
		return NULL;
	return src->v;
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

const char *yz_type_err_ptr(yz_val *val)
{
	printf("|v PTR: level: %d\n", ptr_extract(val->v, NULL));
	return "AMC_PTR";
}
