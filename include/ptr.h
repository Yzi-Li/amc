#ifndef AMC_PTR_H
#define AMC_PTR_H
#include "val.h"

typedef struct yz_ptr {
	yz_val ref;
} yz_ptr;

typedef struct yz_ptr_type {
	yz_type ref;
	int level;
} yz_ptr_type;

yz_type *yz_type_max_ptr(yz_type *lraw, yz_type *rraw, yz_type *l, yz_type *r);

void free_yz_ptr(yz_ptr *self);
void free_yz_ptr_type(yz_ptr_type *self);

#endif
