#ifndef AMC_ARRAY_H
#define AMC_ARRAY_H
#include "type.h"

typedef struct yz_array {
	yz_val type;
	// when len == 0: mut array
	int len;
} yz_array;

const char *yz_type_err_array(yz_val *v);

#endif
