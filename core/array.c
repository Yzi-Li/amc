#include "../include/array.h"

yz_type *yz_type_max_arr(yz_type *l, yz_type *r)
{
	yz_array *larr = l->v, *rarr = r->v;
	if (larr == NULL || rarr == NULL)
		return NULL;
	return yz_type_max(&larr->type, &rarr->type)
		== &larr->type ? l : r;
}
