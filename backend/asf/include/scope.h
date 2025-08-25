/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_SCOPE_H
#define AMC_BE_ASF_SCOPE_H
#include "cond.h"
#include "loop.h"
#include "stack.h"

union asf_scope_status_data {
	struct asf_cond_handle cond;
	struct asf_loop_handle loop;
};

struct asf_scope_status {
	union asf_scope_status_data data;
	struct object_node *end_node, *start_node;
	int identifier_count;
	struct asf_stack_element *stack_start;
	enum {
		ASF_SCOPE_STATUS_NO,
		ASF_SCOPE_STATUS_NORMAL,
		ASF_SCOPE_STATUS_COND,
		ASF_SCOPE_STATUS_LOOP
	} type;
};

#endif
