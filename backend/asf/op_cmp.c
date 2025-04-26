#include "include/asf.h"
#include "include/cmp.h"
#include "include/imm.h"
#include "include/jmp.h"
#include "include/label.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/suffix.h"
#include "../../include/type.h"

static int cmp_and_jmp(struct expr *e, enum ASF_JMP_TYPE jmp_type);

int cmp_and_jmp(struct expr *e, enum ASF_JMP_TYPE jmp_type)
{
	label_id label = -1;
	str *label_str = NULL;
	struct object_node *node = NULL;
	if ((label = asf_label_alloc()) == -1)
		goto err_label_alloc_failed;
	if (asf_inst_cmp(e))
		return 1;
	if ((label_str = asf_label_get_str(label)) == NULL)
		goto err_label_get_str_failed;
	if (asf_inst_jmp(jmp_type, label_str->s, label_str->len))
		goto err_inst_jmp;
	node = malloc(sizeof(*node));
	node->s = label_str;
	if (asf_block_append(node))
		goto err_free_node;
	node->s->len -= 1;
	str_append(node->s, 2, ":\n");
	return 0;
err_label_alloc_failed:
	printf("amc[backend.asf]: cmp_and_jmp: Label alloc failed!\n");
	return 1;
err_label_get_str_failed:
	printf("amc[backend.asf]: cmp_and_jmp: Label get str failed!\n");
	return 1;
err_inst_jmp:
	printf("amc[backend.asf]: cmp_and_jmp: Instruction 'jmp' failed!\n");
	str_free(label_str);
	return 1;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_op_eq(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_NE);
}

int asf_op_ge(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_LT);
}

int asf_op_gt(struct expr *e)
{
	if (YZ_IS_DIGIT(e->vall->type))
		return cmp_and_jmp(e, ASF_JMP_GE);
	return cmp_and_jmp(e, ASF_JMP_LE);
}

int asf_op_le(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_GT);
}

int asf_op_lt(struct expr *e)
{
	if (YZ_IS_DIGIT(e->vall->type))
		return cmp_and_jmp(e, ASF_JMP_LE);
	return cmp_and_jmp(e, ASF_JMP_GE);
}

int asf_op_ne(struct expr *e)
{
	return cmp_and_jmp(e, ASF_JMP_EQ);
}
