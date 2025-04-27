#include "include/asf.h"
#include "include/jmp.h"
#include "include/label.h"

static int loop_gen_end(label_id label);

int loop_gen_end(label_id label)
{
	str *label_str = NULL;
	struct object_node *node = malloc(sizeof(*node));
	if (asf_block_append(node))
		goto err_free_node;
	if ((label_str = asf_label_get_str(label)) == NULL)
		goto err_label_get_str_failed;
	if ((node->s = asf_inst_jmp(ASF_JMP_ALWAYS, label_str->s,
					label_str->len)) == NULL)
		goto err_inst_failed;
	str_free(label_str);
	return 0;
err_free_node:
	free(node);
	return 1;
err_label_get_str_failed:
	printf("amc[backend.asf:%s]: while_gen_end: Label get str failed!\n",
			__FILE__);
	goto err_free_node;
err_inst_failed:
	printf("amc[backend.asf:%s]: while_gen_end: "
			"Get instruction failed!\n", __FILE__);
	str_free(label_str);
	goto err_free_node;
}

int asf_while_begin()
{
	label_id label = -1;
	struct object_node *node = NULL;
	if ((label = asf_label_alloc()) == -1)
		goto err_label_alloc_failed;
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_label_get_str(label)) == NULL)
		goto err_label_get_str_failed;
	node->s->len -= 1;
	str_append(node->s, 2, ":\n");
	return loop_gen_end(label);
err_label_alloc_failed:
	printf("amc[backend.asf:%s]: asf_while_begin: Label alloc failed!\n",
			__FILE__);
	return 1;
err_label_get_str_failed:
	printf("amc[backend.asf:%s]: asf_while_begin: Label get str failed!\n",
			__FILE__);
err_free_node:
	free(node);
	return 1;
}

int asf_while_end()
{
	struct object_node *last_node = NULL;
	if (asf_block_end())
		goto err_block_end_failed;
	last_node = objs[cur_obj][ASF_OBJ_TEXT].last;
	if (object_swap(last_node->prev, last_node))
		goto err_swap_node_failed;
	objs[cur_obj][ASF_OBJ_TEXT].last = last_node->next;
	return 0;
err_block_end_failed:
	printf("amc[backend.asf:%s]: asf_while_end: "
			"Block end failed!\n", __FILE__);
	return 1;
err_swap_node_failed:
	printf("amc[backend.asf:%s]: asf_while_end: "
			"Swap node failed!\n", __FILE__);
	return 1;
}
