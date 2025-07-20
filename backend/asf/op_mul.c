#include "include/asf.h"
#include "include/op.h"
#include "include/suffix.h"
#include <stdlib.h>
#include <string.h>

static const char *temp_signed   = "imul%c %s\n";
static const char *temp_unsigned = "mul%c %s\n";

int asf_op_mul(struct expr *e)
{
	struct object_node *node = NULL;
	const char *temp = YZ_IS_UNSIGNED_DIGIT(e->sum_type->type)
		? temp_unsigned : temp_signed;
	str *tmp = NULL;
	str *multiplicand_str = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((multiplicand_str = asf_op_get_val_right(node, e, ASF_REG_RCX))
			== NULL)
		goto err_free_node;
	if ((tmp = asf_op_get_val_left(node, e)) == NULL)
		goto err_free_node;
	str_free(tmp);
	str_expand(node->s, strlen(temp) - 3
			+ multiplicand_str->len);
	str_append(multiplicand_str, 1, "\0");
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_yz_type2bytes(e->sum_type)),
			multiplicand_str->s);
	str_free(multiplicand_str);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
