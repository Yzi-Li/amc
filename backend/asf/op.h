#ifndef AMC_BE_ASF_OP_H
#define AMC_BE_ASF_OP_H
#include "register.h"
#include "../../include/type.h"
#include "../../include/backend/target.h"

int asf_op_cmp(struct object_node *node, yz_val *l, yz_val *r);
int asf_op_save_reg(struct object_node *parent, enum ASF_REGS reg);
int asf_op_save_val(struct object_node *parent, yz_val *v,
		enum ASF_REGS r);
int asf_op_try_save_val(struct object_node *parent, yz_val *src,
		enum ASF_REGS *dest);

#endif
