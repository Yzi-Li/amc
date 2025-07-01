#include "../include/module.h"

int module_append_child(yz_module *src, yz_module *dest)
{
	dest->children.count += 1;
	dest->children.modules = realloc(dest->children.modules,
			sizeof(*dest->children.modules) * dest->children.count);
	dest->children.modules[dest->children.count - 1] = src;
	return 0;
}
