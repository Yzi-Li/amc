#ifndef AMC_MODULE_H
#define AMC_MODULE_H
#include "symbol.h"

struct yz_module_group {
	struct yz_module **modules;
	int count;
};

struct yz_module;
typedef struct yz_module {
	char *name;
	int name_len;

	struct scope *scope;
	struct yz_module_group children,
			       imported,
			       imported_pub;
} yz_module;

int module_append_child(yz_module *src, yz_module *dest);

#endif
