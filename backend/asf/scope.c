/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/scope.h"
#include "../../include/backend/object.h"
#include <stdlib.h>

static int scope_end_normal(struct asf_scope_status *status);

int scope_end_normal(struct asf_scope_status *status)
{
	asf_stack_end_frame(status->stack_start);
	if (status->end_node == NULL)
		return 0;
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], status->end_node))
		goto err_free_node;
	status->end_node = NULL;
	status->type = ASF_SCOPE_STATUS_NO;
	return 0;
err_free_node:
	str_free(status->end_node->s);
	free(status->end_node);
	return 1;
}

backend_scope_status *asf_scope_begin()
{
	struct asf_scope_status *status = calloc(1, sizeof(*status));
	status->stack_start = asf_stack_top;
	status->type = ASF_SCOPE_STATUS_NO;
	return status;
}

int asf_scope_end(backend_scope_status *raw_status)
{
	struct asf_scope_status *status = raw_status;
	switch (status->type) {
	case ASF_SCOPE_STATUS_NO:
		return 0;
		break;
	case ASF_SCOPE_STATUS_NORMAL:
		if (scope_end_normal(status))
			return 1;
		break;
	case ASF_SCOPE_STATUS_COND:
		if (asf_cond_handle_end(&status->cond))
			return 1;
		if (scope_end_normal(status))
			return 1;
		break;
	case ASF_SCOPE_STATUS_LOOP:
		if (scope_end_normal(status))
			return 1;
		if (asf_loop_handle_end(&status->loop))
			return 1;
		break;
	default:
		goto err_unsupport_type;
		break;
	}
	return 0;
err_unsupport_type:
	printf("amc[backend.asf]: asf_scope_end: "
			"Unsupport status type: \"%d\"\n",
			status->type);
	free(raw_status);
	return 1;
}
