#ifndef AMC_BE_ASF_SCOPE_H
#define AMC_BE_ASF_SCOPE_H
#include "cond.h"
#include "stack.h"

struct asf_scope_status {
	enum {
		ASF_SCOPE_STATUS_NORMAL,
		ASF_SCOPE_STATUS_COND
	} type;
	struct asf_cond_handle cond;
	struct object_node *end_node;
	int identifier_count;
	struct asf_stack_element *stack_start;
};

#endif
