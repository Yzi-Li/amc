#ifndef AMC_BE_IDENTIFIER_H
#define AMC_BE_IDENTIFIER_H
#include "stack.h"
#include "../../../include/backend/symbol.h"

int asf_identifier_reg(backend_symbol_status **raw_sym_stat,
		struct asf_stack_element *src);

#endif
