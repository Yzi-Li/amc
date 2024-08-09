#include "asf.h"
#include "imm.h"
#include "op.h"
#include "suffix.h"
#include "../../utils/utils.h"

static const char *temp_signed   = "mul%c %s\n";
static const char *temp_unsigned = "imul%c %s, %s\n";

static int asf_op_imul(struct object_node *node, yz_val *l, yz_val *r);

int asf_op_imul(struct object_node *node, yz_val *l, yz_val *r)
{
	const char *temp = "imul%c %s, %s\n";
	enum ASF_REGS multiplier_reg = ASF_REG_RDX,
	              result_reg     = ASF_REG_RAX;
	str *multiplier_str = NULL,
	    *result_str     = NULL;
	enum YZ_TYPE sign_left_type  = l->type,
		     sign_right_type = r->type;
	if (l->type == AMC_SUB_EXPR && r->type == AMC_SUB_EXPR) {
		str_append(node->s, 26,
				"imulq (%rsp)\n"
				"addq $8, %rsp\n");
		return 0;
	}
	if (REGION_INT(l->type, YZ_U8, YZ_U64))
		sign_left_type = l->type - 4;
	if (REGION_INT(r->type, YZ_U8, YZ_U64))
		sign_right_type = r->type - 4;
	result_reg = asf_reg_get(sign_left_type);
	multiplier_reg = asf_reg_get(sign_right_type) + ASF_REG_RDX;
	asf_op_save_val(node, l, result_reg);
	asf_op_save_val(node, r, multiplier_reg);
	multiplier_str = asf_reg_get_str(&regs[multiplier_reg]);
	result_str = asf_reg_get_str(&regs[result_reg]);
	str_expand(node->s, strlen(temp)
			+ multiplier_str->len
			+ result_str->len);
	str_append(multiplier_str, 1, "\0");
	str_append(result_str, 1, "\0");
	snprintf(node->s->s, node->s->len, temp,
			suffix_get(regs[result_reg].size),
			multiplier_str->s,
			result_str->s);
	str_free(multiplier_str);
	str_free(result_str);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_op_mul(yz_val *l, yz_val *r)
{
	struct asf_imm imm = {};
	struct object_node *node = NULL;
	enum ASF_REGS multiplier_reg = ASF_REG_RDX,
	              result_reg     = ASF_REG_RAX;
	str *multiplier_str = NULL;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(objs[ASF_OBJ_TEXT], node)) {
		str_free(node->s);
		free(node);
		return 1;
	}
	if (REGION_INT(MIN(l->type, r->type), YZ_I8, YZ_I64))
		return asf_op_imul(node, l, r);
	if (l->type == AMC_SUB_EXPR && r->type == AMC_SUB_EXPR) {
		str_append(node->s, 26,
				"mulq (%rsp)\n"
				"addq $8, %rsp\n");
		return 0;
	}
	if (l->type != AMC_SUB_EXPR) {
		if (regs[result_reg].flags.used)
			asf_op_save_reg(node, &regs[result_reg]);
		asf_op_save_val(node, l, result_reg);
	}
	if (r->type != AMC_SUB_EXPR) {
		if (regs[multiplier_reg].flags.used)
			asf_op_save_reg(node, &regs[multiplier_reg]);
		asf_op_save_val(node, r, multiplier_reg);
	}
	multiplier_str = asf_reg_get_str(&regs[multiplier_reg]);
	str_expand(node->s, strlen(temp_signed)
			+ multiplier_str->len);
	str_append(multiplier_str, 1, "\0");
	snprintf(node->s->s, node->s->len, temp_signed,
			suffix_get(regs[multiplier_reg].size),
			multiplier_str->s);
	str_free(multiplier_str);
	return 0;
}
