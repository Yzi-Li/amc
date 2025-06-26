#ifndef AMC_BE_ASF_OP_VAL_H
#define AMC_BE_ASF_OP_VAL_H
#include "../../../include/expr.h"
#include "../../../include/type.h"

int asf_op_extract_array_elem(yz_extract_val *val);
struct asf_stack_element *asf_op_extract_get_mem(yz_extract_val *val);
int asf_op_extract_ptr_val(struct symbol *sym);
int asf_op_extract_ptr_val_from_expr(struct expr *expr);
int asf_op_extract_struct_elem(yz_extract_val *val);

#endif
