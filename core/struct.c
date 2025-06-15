#include "../include/struct.h"
#include "../include/symbol.h"
#include "../utils/utils.h"

//TODO
void struct_free(yz_struct *src)
{
	symbol_group_free(src->elems, src->elem_count);
	free_safe(src->name);
	free_safe(src);
}
