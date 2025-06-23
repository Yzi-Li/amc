#include "include/constructor.h"
#include "../include/type.h"

void constructor_handle_free(struct constructor_handle *handle)
{
	for (int i = 0; i < handle->len; i++) {
		if (handle->vs[i] == NULL)
			return;
		free_yz_val(handle->vs[i]);
	}
	free(handle);
}
