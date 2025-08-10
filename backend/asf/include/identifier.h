/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_IDENTIFIER_H
#define AMC_BE_ASF_IDENTIFIER_H
#include "stack.h"
#include "../../../include/backend/symbol.h"
#include "../../../include/op.h"

int asf_identifier_reg(backend_symbol_status **raw_sym_stat,
		struct asf_stack_element *src);
str *asf_identifier_set(struct asf_stack_element *dest, enum OP_ID mode,
		yz_val *src);

#endif
