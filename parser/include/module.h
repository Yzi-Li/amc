#ifndef AMC_PARSER_MODULE_H
#define AMC_PARSER_MODULE_H
#include "../../utils/str/str.h"

static const char *MODULE_NAME_VAILD_CHARS =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"_";

int test_module_name(str *name);

#endif
