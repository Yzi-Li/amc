/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/bytes.h"
#include "include/op.h"
#include "include/suffix.h"
#include <stdlib.h>
#include <string.h>

static const char *temp = "add%c %s, %s\n";

int asf_op_add(struct expr *e)
{
	str *addend = NULL,
	    *augend = NULL;
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((addend = asf_op_get_val_right(node, e, -1)) == NULL)
		goto err_free_node;
	if ((augend = asf_op_get_val_left(node, e)) == NULL)
		goto err_free_node_and_addend;
	str_expand(node->s, strlen(temp) - 4
			+ addend->len - 1
			+ augend->len - 1);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_yz_type2bytes(e->sum_type)),
			addend->s,
			augend->s);
	str_free(addend);
	str_free(augend);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
err_free_node_and_addend:
	str_free(addend);
	goto err_free_node;
}
