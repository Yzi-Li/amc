/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/stack.h"
#include "../../include/backend/object.h"
#include <stdlib.h>

struct asf_scope_status {
	struct object_node *end_node, *start_node;
	int identifier_count;
	struct asf_stack_element *stack_start;
};

backend_scope_status *asf_scope_begin(void)
{
	struct asf_scope_status *status = calloc(1, sizeof(*status));
	status->start_node = cur_obj->sections[ASF_OBJ_TEXT].last;
	status->stack_start = asf_stack_top;
	return status;
}

int asf_scope_end(backend_scope_status *raw_status)
{
	struct asf_scope_status *status = raw_status;
	return asf_stack_end_frame(status->stack_start);
}

void asf_scope_free(backend_scope_status *raw_status)
{
	struct asf_scope_status *self = raw_status;
	if (raw_status == NULL)
		return;
	free(self);
}
