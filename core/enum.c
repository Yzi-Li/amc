/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/enum.h"

void free_yz_enum(yz_enum *self)
{
	if (self == NULL)
		return;
	for (int i = 0; i < self->count; i++)
		free_yz_enum_item(self->elems[i]);
	str_free_noself(&self->name);
	free_yz_type_noself(&self->type);
	free(self->elems);
	free(self);
}

void free_yz_enum_item(yz_enum_item *self)
{
	if (self == NULL)
		return;
	str_free_noself(&self->name);
	free(self);
}
