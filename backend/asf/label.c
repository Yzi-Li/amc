#include "include/asf.h"
#include "include/label.h"
#include "../../utils/utils.h"
#include <stdio.h>

struct block_node {
	struct object_node *node;
	struct block_node *prev;
};

static struct block_node *cur_block;
static int label_id_top = -1;

static void blocks_free();
static int label_is_exists(label_id id);

static void blocks_free()
{
	struct block_node *next = NULL;
	while (cur_block != NULL) {
		next = cur_block->prev;
		str_free(cur_block->node->s);
		free_safe(cur_block->node);
		free(cur_block);
		cur_block = next;
	}
}

int label_is_exists(label_id id)
{
	if (id > label_id_top)
		return 0;
	return 1;
}

int asf_block_end()
{
	struct block_node *next = NULL;
	if (cur_block == NULL)
		goto err_no_block;
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], cur_block->node))
		goto err_free_node;
	next = cur_block->prev;
	free(cur_block);
	cur_block = next;
	return 0;
err_no_block:
	printf("amc[backend.asf]: asf_block_end: No block!\n");
	return 1;
err_free_node:
	blocks_free();
	return 1;
}

int asf_block_append(struct object_node *node)
{
	struct block_node *block = malloc(sizeof(*block));
	block->node = node;
	if (cur_block == NULL) {
		block->prev = NULL;
	} else {
		block->prev = cur_block;
	}
	cur_block = block;
	return 0;
}

label_id asf_label_alloc()
{
	label_id_top += 1;
	return label_id_top;
}

str *asf_label_get_str(label_id id)
{
	str *s = NULL;
	const char *temp = ".L%lld";
	if (!label_is_exists(id))
		goto err_label_not_found;
	s = str_new();
	str_expand(s, strlen(temp) - 3 + ullen(id));
	snprintf(s->s, s->len, temp, id);
	return s;
err_label_not_found:
	printf("amc[backend.asf]: asf_label_get_str: Label not found!\n");
	return NULL;
}
