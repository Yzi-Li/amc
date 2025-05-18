#ifndef AMC_PTR_H
#define AMC_PTR_H
#include "symbol.h"
#include "type.h"

typedef struct yz_ptr {
	yz_val ref;
} yz_ptr;

int parse_ptr(str *token, yz_val *ptr);
const char *yz_err_ptr_type(yz_val *val);
yz_ptr *yz_ptr_get_from_val(yz_val *val);
int yz_ptr_is_equal(yz_ptr *l, yz_ptr *r);

#endif
