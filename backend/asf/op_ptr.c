#include "include/asf.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/stack.h"
#include "include/suffix.h"
#include "../../include/ptr.h"

static int op_ptr_extract_val_get_addr(enum ASF_REGS dest, struct symbol *sym);
static str *op_ptr_identifier_get(yz_ptr *ptr);

int op_ptr_extract_val_get_addr(enum ASF_REGS dest, struct symbol *sym)
{
	char *name = NULL;
	struct object_node *node = NULL;
	struct asf_stack_element *src = NULL;
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	name = str2chr(sym->name, sym->name_len);
	src = asf_identifier_get(name);
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, src, &dest)) == NULL)
		goto err_inst_failed;
	return 0;
err_free_node:
	free(node);
	return 1;
err_inst_failed:
	printf("amc[backend.asf:%s]: op_ptr_extract_val_get_addr: "
			"Get instruction failed!\n", __FILE__);
	goto err_free_node;
}

str *op_ptr_identifier_get(yz_ptr *ptr)
{
	struct symbol *sym = ptr->ref.v;
	char *name = str2chr(sym->name, sym->name_len);
	struct asf_stack_element *stack = asf_identifier_get(name);
	str *result = asf_stack_get_element(stack, 0);
	if (result->s[result->len - 1] != '\0')
		str_append(result, 1, "\0");
	return result;
}

int asf_op_extract_val(struct expr *e)
{
	enum ASF_REGS dest = ASF_REG_RAX, src = ASF_REG_RAX;
	struct object_node *node = NULL;
	yz_ptr *ptr = NULL;
	const char *temp = "mov%c (%%%s), %%%s\n";
	if (e->valr->type != AMC_SYM)
		return 1;
	if (op_ptr_extract_val_get_addr(src, e->valr->v))
		return 1;
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	ptr = ((struct symbol*)e->valr->v)->result_type.v;
	dest = asf_reg_get(asf_yz_type_raw2imm(ptr->ref.type));
	str_expand(node->s, strlen(temp));
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_regs[dest].size),
			asf_regs[src].name,
			asf_regs[dest].name);
	return 0;
err_free_node:
	free(node);
	return 1;
}

int asf_op_get_addr(struct expr *e)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	str *identifier = NULL;
	struct object_node *node = NULL;
	const char *temp = "lea%c %s, %%%s\n";
	if (e->valr->type != YZ_PTR)
		return 1;
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	dest = asf_reg_get(asf_yz_type_raw2imm(*e->sum_type));
	if (*asf_regs[dest].purpose != ASF_REG_PURPOSE_NULL)
		if (asf_op_save_reg(node, dest))
			goto err_free_node;
	if ((identifier = op_ptr_identifier_get(e->valr->v)) == NULL)
		goto err_free_node;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 3 + identifier->len);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_regs[dest].size),
			identifier->s,
			asf_regs[dest].name);
	return 0;
err_free_node:
	free(node);
	return 1;
}
