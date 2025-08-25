/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/jmp.h"
#include "include/label.h"
#include "../../include/backend/object.h"
#include <stdlib.h>

struct while_handle {
	label_id begin_label, cond_label;
};

static int loop_append_jmp_begin(label_id label);
static int loop_append_label(label_id label);

int loop_append_jmp_begin(label_id label)
{
	str *label_str = NULL;
	struct object_node *node = malloc(sizeof(*node));
	label_str = asf_label_get_str(label);
	node->s = asf_inst_jmp(ASF_JMP_ALWAYS, label_str->s, label_str->len);
	str_free(label_str);
	if (node->s == NULL)
		goto err_free_node;
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return 1;
}

int loop_append_label(label_id label)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = asf_label_get_str(label);
	node->s->len -= 1;
	str_append(node->s, 2, ":\n");
	return 0;
err_free_node:
	free(node);
	return 1;
}

backend_while_handle *asf_while_begin(void)
{
	struct while_handle *result = NULL;
	label_id label = asf_label_alloc();
	if (loop_append_label(label))
		return NULL;
	result = calloc(1, sizeof(*result));
	result->begin_label = label;
	return result;
}

int asf_while_cond(backend_while_handle *handle)
{
	struct while_handle *h = handle;
	if (h == NULL)
		return 1;
	h->cond_label = asf_label_get_last();
	return 0;
}

int asf_while_end(backend_while_handle *handle)
{
	struct while_handle *h = handle;
	if (h == NULL)
		return 1;
	if (loop_append_jmp_begin(h->begin_label))
		return 1;
	if (loop_append_label(h->cond_label))
		return 1;
	asf_while_free_handle(handle);
	return 0;
}

void asf_while_free_handle(backend_while_handle *handle)
{
	if (handle == NULL)
		return;
	free(handle);
}
