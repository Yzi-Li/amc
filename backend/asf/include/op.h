/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_OP_H
#define AMC_BE_ASF_OP_H
#include "register.h"
#include "val.h"
#include "../../../include/expr.h"
#include "../../../include/backend/object.h"

#define ASF_OP_OPERAND_REG ASF_REG_RBX
#define ASF_OP_RESULT_REG ASF_REG_RAX

int asf_op_clean_reg(struct object_node *parent, enum ASF_REGS reg);
str *asf_op_get_dest(enum ASF_BYTES *bytes, struct asf_val *dest);
str *asf_op_get_src(enum ASF_BYTES *bytes, struct asf_val *src);
int asf_op_handle_expr(str **result, struct expr *e, enum ASF_REGS *src,
		enum ASF_REGS dest);
str *asf_op_handle_expr_and_expr(enum ASF_REGS *src, enum ASF_REGS dest);
int asf_op_save_reg(struct object_node *parent, enum ASF_REGS reg);
int asf_op_store_val(yz_val *val, enum ASF_REGS *dest);

/**
 * If the result register store the prev expression result,
 * this function will push it to stack. Then return 1.
 * In other cases, do nothing.
 */
enum TRY_RESULT asf_op_try_push_prev_expr_result(struct expr *e,
		enum ASF_REGS reg);

str *asf_inst_op_add(struct asf_val *src, struct asf_val *dest);

/**
 * Divides the decimal to rAX.
 * Before call this instruction, please store the dividend to
 * AX, DX:AX, EDX:EAX, RDX:RAX.
 * And the high-order bits will store in AX(Special), DX, EDX, RDX.
 * So you **must** clean or store a decimal to these registers.
 * And the result will store in AL,AX,EAX,RAX.
 * The remainder will store in AH,DX,EDX,RDX.
 *
 * @param is_unsigned:
 *   0: signed
 *   1: unsigned
 *   2: auto
 */
str *asf_inst_op_div(struct asf_val *src, int is_unsigned);

/**
 * Multiplies the decimal to rAX.
 * Before call this instruction, please store the multiplier(AL,AX,EAX,or RAX).
 * And the result will store in AX, DX:AX, EDX:EAX, RDX:RAX.
 * It puts the high-order bits of the product in AH,DX,EDX,or RDX.
 *
 * @param is_unsigned:
 *   0: signed
 *   1: unsigned
 *   2: auto
 */
str *asf_inst_op_mul(struct asf_val *src, int is_unsigned);

str *asf_inst_op_sub(struct asf_val *src, struct asf_val *dest);

#endif
