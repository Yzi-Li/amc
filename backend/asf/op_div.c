#include "asf.h"
#include "inst.h"
#include "op.h"
#include "register.h"
#include "suffix.h"

static const char *temp_signed   = "idiv%c %s\n";
static const char *temp_unsigned = "div%c %s\n";

static int div_expr_and_expr(struct object_node *node, struct expr *e);

int div_expr_and_expr(struct object_node *node, struct expr *e)
{
	const char *temp = YZ_IS_UNSIGNED_DIGIT(*e->sum_type)
		? temp_unsigned : temp_signed;
	enum ASF_REGS divisor_reg = ASF_REG_RCX,
	              result_reg  = ASF_REG_RAX;
	str *divisor_reg_str = NULL,
	    *get_divisor     = NULL,
	    *inst            = str_new(),
	    *pop_result      = NULL;
	result_reg = asf_reg_get(asf_yz_type2imm(*e->sum_type));
	divisor_reg += result_reg;
	divisor_reg_str = asf_reg_get_str(&asf_regs[divisor_reg]);
	get_divisor = asf_inst_mov(ASF_MOV_R2R, &result_reg, &divisor_reg);
	pop_result = asf_inst_pop(result_reg);
	str_append(node->s, get_divisor->len - 1, get_divisor->s);
	str_append(node->s, pop_result->len - 1, pop_result->s);
	str_expand(inst, strlen(temp) - 1
			+ divisor_reg_str->len);
	snprintf(inst->s, inst->len, temp,
			asf_suffix_get(asf_regs[divisor_reg].size),
			divisor_reg_str->s);
	str_append(node->s, inst->len - 1, inst->s);
	asf_regs[result_reg].flags.used = 1;
	*asf_regs[result_reg].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	str_free(divisor_reg_str);
	str_free(get_divisor);
	str_free(inst);
	str_free(pop_result);
	return 0;
}

int asf_op_div(struct expr *e)
{
	enum ASF_REGS divisor_reg = ASF_REG_RCX,
	              remainder   = ASF_REG_RDX,
	              result_reg  = ASF_REG_RAX;
	str *divisor_reg_str = NULL;
	struct object_node *node = NULL;
	const char *temp = temp_signed;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (asf_op_clean_reg(node, remainder))
		goto err_free_node;
	if (e->vall->type == AMC_EXPR && e->valr->type == AMC_EXPR)
		return div_expr_and_expr(node, e);
	if (asf_op_try_save_val(node, e->valr, &divisor_reg))
		goto err_free_node;
	if (asf_op_try_save_val(node, e->vall, &result_reg))
		goto err_free_node;
	divisor_reg_str = asf_reg_get_str(&asf_regs[divisor_reg]);
	if (YZ_IS_UNSIGNED_DIGIT(*e->sum_type))
		temp = temp_unsigned;
	str_expand(node->s, strlen(temp) - 1 + divisor_reg_str->len);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_regs[divisor_reg].size),
			divisor_reg_str->s);
	*asf_regs[divisor_reg].purpose = ASF_REG_PURPOSE_NULL;
	asf_regs[divisor_reg].flags.used = 0;
	*asf_regs[result_reg].purpose = ASF_REG_PURPOSE_EXPR_RESULT;
	asf_regs[result_reg].flags.used = 1;
	str_free(divisor_reg_str);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
