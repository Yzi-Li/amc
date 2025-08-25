/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/label.h"
#include "../../include/backend/object.h"
#include "../../utils/utils.h"
#include <stdint.h>
#include <string.h>

int asf_null_handle_begin(backend_null_handle **handle, yz_val *val)
{
	label_id label;
	struct object_node *node;
	const char *temp =
		"cmpq $0, %%rax\n"
		"jne .L%d\n";
	if (val->type.type != AMC_SYM || val->data.sym->type != SYM_FUNC)
		return 1;
	label = asf_label_alloc();
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 2 + ullen(label));
	snprintf(node->s->s, node->s->len, temp, label);
	*(intptr_t*)handle = label;
	return 0;
err_free_node:
	free(node);
	return 1;
}

int asf_null_handle_end(backend_null_handle *handle)
{
	label_id label = (intptr_t)handle;
	struct object_node *node = malloc(sizeof(*node));
	const char *temp = ".L%d:\n";
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 1 + ullen(label));
	snprintf(node->s->s, node->s->len, temp, label);
	return 0;
err_free_node:
	free(node);
	return 1;
}
