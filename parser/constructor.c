/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/constructor.h"

void free_constructor_handle(struct constructor_handle *handle)
{
	for (int i = 0; i < handle->len; i++) {
		if (handle->vs[i] == NULL)
			return;
		free_yz_val(handle->vs[i]);
	}
	free(handle->vs);
	free(handle);
}
