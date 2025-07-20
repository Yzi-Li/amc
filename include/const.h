#ifndef AMC_CONST_H
#define AMC_CONST_H
#include "backend/const.h"
#include "val.h"

typedef struct yz_const {
	backend_const be_data;
	yz_val val;
} yz_const;

#endif
