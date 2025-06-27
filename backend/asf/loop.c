#include "include/asf.h"
#include "include/jmp.h"
#include "include/label.h"
#include "include/scope.h"
#include <stdlib.h>

static int loop_append_jmp_begin(struct asf_scope_status *status,
		label_id label);
static int loop_append_label(label_id label);

int loop_append_jmp_begin(struct asf_scope_status *status, label_id label)
{
	str *label_str = NULL;
	struct object_node *node = malloc(sizeof(*node));
	label_str = asf_label_get_str(label);
	if ((node->s = asf_inst_jmp(ASF_JMP_ALWAYS, label_str->s,
					label_str->len)) == NULL)
		goto err_free_node;
	status->end_node = node;
	return 0;
err_free_node:
	str_free(label_str);
	free(node);
	return 1;
}

int loop_append_label(label_id label)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = asf_label_get_str(label);
	node->s->len -= 1;
	str_append(node->s, 2, ":\n");
	return 0;
err_free_node:
	free(node);
	return 1;
}

int asf_while_begin(backend_scope_status *raw_status)
{
	label_id label = -1;
	struct asf_scope_status *status = raw_status;
	if (status->type != ASF_SCOPE_STATUS_NO)
		if (asf_scope_end(raw_status))
			return 1;
	label = asf_label_alloc();
	if (loop_append_label(label))
		return 1;
	if (loop_append_jmp_begin(status, label))
		return 1;
	return 0;
}

int asf_while_end(backend_scope_status *raw_status)
{
	if (asf_scope_end(raw_status))
		return 1;
	if (loop_append_label(asf_label_get_last()))
		return 1;
	return 0;
}
