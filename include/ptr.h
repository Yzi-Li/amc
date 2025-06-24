#ifndef AMC_PTR_H
#define AMC_PTR_H
#include "type.h"

typedef struct yz_ptr {
	yz_val ref;
} yz_ptr;

yz_ptr *yz_ptr_get_from_val(yz_val *val);
yz_ptr *yz_ptr_get_from_val_extracted(yz_extract_val *val, yz_val **type);
int yz_ptr_is_equal(yz_ptr *l, yz_ptr *r);
const char *yz_type_err_ptr(yz_val *val);

#endif
