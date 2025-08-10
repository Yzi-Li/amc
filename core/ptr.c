/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/ptr.h"
#include "../include/type.h"

yz_type *yz_type_max_ptr(yz_type *lraw, yz_type *rraw, yz_type *l, yz_type *r)
{
	yz_ptr_type *lptr = lraw->v, *rptr = rraw->v;
	if (lptr == NULL || rptr == NULL)
		return NULL;
	if (lptr->level != rptr->level)
		return NULL;
	return yz_type_max(&lptr->ref, &rptr->ref)
		== &lptr->ref ? l : r;
}

void free_yz_ptr(yz_ptr *self)
{
	free_yz_val_noself(&self->ref);
	free(self);
}

void free_yz_ptr_type(yz_ptr_type *self)
{
	free_yz_type_noself(&self->ref);
	free(self);
}
