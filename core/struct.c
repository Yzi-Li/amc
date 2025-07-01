#include "../include/struct.h"
#include "../include/symbol.h"
#include "../utils/utils.h"

void free_struct(yz_struct *src)
{
	free_symbol_group(src->elems, src->elem_count);
	free_safe(src->name);
	free_safe(src);
}

void free_structs(yz_struct **elems, int count)
{
	for (int i = 0; i < count; i++) {
		free_struct(elems[i]);
	}
}
