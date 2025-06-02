#include "include/asf.h"
#include "include/cmp.h"
#include "include/jmp.h"
#include "include/label.h"

static int cmp_and_jmp(struct expr *e, enum ASF_JMP_TYPE jmp_type);
static int cmp_jmp_inst_append(label_id label, str *label_str,
		enum ASF_JMP_TYPE jmp_type);

int cmp_and_jmp(struct expr *e, enum ASF_JMP_TYPE jmp_type)
{
	label_id label = -1;
	str *label_str = NULL;
	if ((label = asf_label_alloc()) == -1)
		goto err_label_alloc_failed;
	if (asf_inst_cmp(e))
		return 1;
	if ((label_str = asf_label_get_str(label)) == NULL)
		goto err_label_get_str_failed;
	if (cmp_jmp_inst_append(label, label_str, jmp_type))
		return 1;
	return 0;
err_label_alloc_failed:
	printf("amc[backend.asf:%s]: cmp_and_jmp: Label alloc failed!\n",
			__FILE__);
	return 1;
err_label_get_str_failed:
	printf("amc[backend.asf:%s]: cmp_and_jmp: Label get str failed!\n",
			__FILE__);
	return 1;
}

int cmp_jmp_inst_append(label_id label, str *label_str,
		enum ASF_JMP_TYPE jmp_type)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_jmp(jmp_type, label_str->s, label_str->len))
			== NULL)
		goto err_inst_jmp;
	return 0;
err_free_node:
	free(node);
	return 1;
err_inst_jmp:
	printf("amc[backend.asf:%s]: cmp_jmp_inst_append: "
			"Get instruction 'jmp' failed!\n", __FILE__);
	str_free(label_str);
	goto err_free_node;
}

int asf_op_eq(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_NE);
}

int asf_op_ne(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_EQ);
}

int asf_op_le(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_GT);
}

int asf_op_lt(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_GE);
}

int asf_op_ge(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_LT);
}

int asf_op_gt(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_LE);
}
