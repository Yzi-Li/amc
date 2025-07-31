#include "../include/enum.h"

void free_yz_enum(yz_enum *self)
{
	if (self == NULL)
		return;
	free_yz_type_noself(&self->type);
}
