#ifndef AMC_BE_ASF_OP_VAL_H
#define AMC_BE_ASF_OP_VAL_H
#include "../../../include/type.h"

int asf_op_extract_array_elem(yz_extract_val *val);
int asf_op_extract_ptr_val(struct symbol *sym);
int asf_op_extract_struct_elem(yz_extract_val *val);

#endif
