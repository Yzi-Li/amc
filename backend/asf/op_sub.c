#include "include/asf.h"
#include "include/op.h"
#include "include/suffix.h"

static const char *temp = "sub%c %s, %s\n";

int asf_op_sub(struct expr *e)
{
	struct object_node *node = NULL;
	str *minuend = NULL,
	    *subtrahend = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((subtrahend = asf_op_get_val_right(node, e, -1)) == NULL)
		goto err_free_node;
	if ((minuend = asf_op_get_val_left(node, e)) == NULL)
		goto err_free_node_and_subtrahend;
	str_expand(node->s, strlen(temp) - 4
			+ subtrahend->len - 1
			+ minuend->len - 1);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_yz_type2imm(*e->sum_type)),
			subtrahend->s,
			minuend->s);
	str_free(minuend);
	str_free(subtrahend);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
err_free_node_and_subtrahend:
	str_free(subtrahend);
	goto err_free_node;
}
