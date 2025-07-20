#include "../include/expr.h"
#include "../include/struct.h"
#include "../include/symbol.h"
#include "../include/val.h"

struct symbol *yz_get_extracted_val(yz_extract_val *val)
{
	switch (val->type) {
	case YZ_EXTRACT_STRUCT:
		return ((yz_struct*)val->sym->result_type.v)
			->elems[val->index];
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

void free_yz_extract_val(struct yz_extract_val *self)
{
	if (self == NULL)
		return;
	switch (self->type) {
	case YZ_EXTRACT_ARRAY:
		free_yz_val(self->offset);
		break;
	default:
		free(self);
		break;
	}
}

void free_yz_val(yz_val *self)
{
	if (self == NULL)
		return;
	free_yz_val_noself(self);
	free(self);
}

void free_yz_val_noself(yz_val *self)
{
	if (self == NULL)
		return;
	free_yz_type_noself(&self->type);
}
