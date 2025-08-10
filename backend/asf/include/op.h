/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_OP_H
#define AMC_BE_ASF_OP_H
#include "register.h"
#include "../../../include/expr.h"
#include "../../../include/backend/object.h"

int asf_op_clean_reg(struct object_node *parent, enum ASF_REGS reg);
str *asf_op_get_val_left(struct object_node *parent, struct expr *e);
str *asf_op_get_val_right(struct object_node *parent, struct expr *e,
		enum ASF_REGS dest);
int asf_op_save_reg(struct object_node *parent, enum ASF_REGS reg);

str *asf_inst_op_add(enum ASF_REGS src, enum ASF_REGS dest);
str *asf_inst_op_div(enum ASF_REGS src, int is_unsigned);
str *asf_inst_op_mul(enum ASF_REGS src, int is_unsigned);
str *asf_inst_op_sub(enum ASF_REGS src, enum ASF_REGS dest);

#endif
