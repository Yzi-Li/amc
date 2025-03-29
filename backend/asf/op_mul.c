#include "asf.h"
#include "inst.h"
#include "imm.h"
#include "op.h"
#include "suffix.h"

static const char *temp_signed   = "imul%c %s\n";
static const char *temp_unsigned = "mul%c %s\n";

static int mul_expr_and_expr(struct object_node *node, struct expr *e);

int mul_expr_and_expr(struct object_node *node, struct expr *e)
{
	char suffix = asf_suffix_get(asf_stack_top->bytes);
	const char *temp = YZ_IS_UNSIGNED_DIGIT(*e->sum_type)
		? temp_unsigned : temp_signed;
	str *multiplicand = asf_stack_get_element(asf_stack_top, 1);
	str_expand(node->s, strlen(temp) - 1
			+ multiplicand->len - 1);
	snprintf(node->s->s, node->s->len, temp,
			suffix,
			multiplicand->s);
	str_free(multiplicand);
	return 0;
}

int asf_op_mul(struct expr *e)
{
	struct object_node *node = NULL;
	const char *temp = YZ_IS_UNSIGNED_DIGIT(*e->sum_type)
		? temp_unsigned : temp_signed;
	enum ASF_REGS multiplicand_reg = ASF_REG_RDX,
	              result_reg       = ASF_REG_RAX;
	str *multiplicand_str = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (e->vall->type == AMC_EXPR && e->valr->type == AMC_EXPR)
		return mul_expr_and_expr(node, e);
	if (asf_op_try_save_val(node, e->vall, &result_reg))
		goto err_free_node;
	if (asf_op_try_save_val(node, e->valr, &multiplicand_reg))
		goto err_free_node;
	multiplicand_str = asf_reg_get_str(&asf_regs[multiplicand_reg]);
	str_expand(node->s, strlen(temp) - 3
			+ multiplicand_str->len);
	str_append(multiplicand_str, 1, "\0");
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_regs[multiplicand_reg].size),
			multiplicand_str->s);
	str_free(multiplicand_str);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
