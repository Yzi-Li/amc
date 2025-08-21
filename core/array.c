/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/array.h"

yz_type *yz_type_max_arr(yz_type *l, yz_type *r)
{
	yz_array_type *larr = l->v, *rarr = r->v;
	if (larr == NULL || rarr == NULL)
		return NULL;
	return yz_type_max(&larr->type, &rarr->type)
		== &larr->type ? l : r;
}

void free_yz_array_type(yz_array_type *self)
{
	free_yz_type_noself(&self->type);
	free(self);
}
