/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/const.h"
#include "../include/backend/const.h"

void free_yz_const(yz_const *self)
{
	if (self == NULL)
		return;
	free_backend_const_noself(&self->be_data);
	free_yz_val_noself(&self->val);
	free(self);
}
