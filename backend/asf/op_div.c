#include "include/asf.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/suffix.h"

static const char *temp_signed   = "idiv%c %s\n";
static const char *temp_unsigned = "div%c %s\n";

int asf_op_div(struct expr *e)
{
	str *divisor = NULL,
	    *tmp = NULL;
	enum ASF_REGS remainder   = ASF_REG_RDX,
	              result_reg  = ASF_REG_RAX;
	struct object_node *node = NULL;
	const char *temp = temp_signed;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (asf_op_clean_reg(node, remainder))
		goto err_free_node;
	if ((divisor = asf_op_get_val_right(node, e, ASF_REG_RCX)) == NULL)
		goto err_free_node;
	if ((tmp = asf_op_get_val_left(node, e)) == NULL)
		goto err_free_node;
	str_free(tmp);
	if (YZ_IS_UNSIGNED_DIGIT(*e->sum_type))
		temp = temp_unsigned;
	str_expand(node->s, strlen(temp) - 1 + divisor->len);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_yz_type2imm(*e->sum_type)),
			divisor->s);
	*asf_regs[result_reg].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	str_free(divisor);
	return 0;
err_free_node:
	str_free(divisor);
	str_free(node->s);
	free(node);
	return 1;
}
