#ifndef AMC_BE_ASF_OP_H
#define AMC_BE_ASF_OP_H
#include "register.h"
#include "../../../include/type.h"
#include "../../../include/backend/target.h"

int asf_op_clean_reg(struct object_node *parent, enum ASF_REGS reg);
str *asf_op_get_val_left(struct object_node *parent, struct expr *e);
str *asf_op_get_val_right(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest);
int asf_op_save_reg(struct object_node *parent, enum ASF_REGS reg);

#endif
