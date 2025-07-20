#include "include/asf.h"
#include "include/bytes.h"
#include "include/call.h"
#include "include/mov.h"
#include "include/op.h"
#include "include/op_val.h"
#include "include/stack.h"
#include "include/struct.h"
#include "include/suffix.h"
#include "../../include/ptr.h"
#include "../../include/symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int op_ptr_extract_from_reg(enum ASF_REGS src, enum ASF_REGS dest);
static int op_ptr_extract_get_addr(enum ASF_REGS *dest, struct symbol *sym);
static str *op_ptr_get_addr_get_src(yz_ptr *ptr);
static str *op_ptr_get_addr_get_src_from_sym(yz_ptr *ptr);

int op_ptr_extract_from_reg(enum ASF_REGS src, enum ASF_REGS dest)
{
	struct object_node *node = NULL;
	const char *temp = "mov%c (%%%s), %%%s\n";
	if (src > ASF_REG_RSP)
		return 1;
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	str_expand(node->s, strlen(temp));
	snprintf(node->s->s, node->s->len , temp,
			asf_suffix_get(asf_regs[dest].bytes),
			asf_regs[src].name,
			asf_regs[dest].name);
	return 0;
err_free_node:
	free(node);
	return 1;
}

int op_ptr_extract_get_addr(enum ASF_REGS *dest, struct symbol *sym)
{
	struct object_node *node = NULL;
	struct asf_stack_element *src = NULL;
	if (sym->type == SYM_FUNC)
		return 1;
	if (sym->type == SYM_FUNC_ARG) {
		if (sym->argc > asf_call_arg_regs_len)
			return 1;
		*dest = asf_call_arg_regs[sym->argc];
		return 0;
	}
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	src = sym->backend_status;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, src, dest)) == NULL)
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

str *op_ptr_get_addr_get_src(yz_ptr *ptr)
{
	str *result = NULL;
	struct asf_stack_element *src = NULL;
	if (ptr->ref.type.type == AMC_SYM)
		return op_ptr_get_addr_get_src_from_sym(ptr);
	if (ptr->ref.type.type != AMC_EXTRACT_VAL)
		goto err_not_expr;
	if ((src = asf_op_extract_get_mem(ptr->ref.v)) == NULL)
		return NULL;
	if ((result = asf_stack_get_element(src, 0)) == NULL)
		return NULL;
	if (result->s[result->len - 1] != '\0')
		str_append(result, 1, "\0");
	return result;
err_not_expr:
	printf("amc[backend.asf:%s]: op_ptr_get_addr_get_src: "
			"Value is not a expression!\n", __FILE__);
	return NULL;
}

str *op_ptr_get_addr_get_src_from_sym(yz_ptr *ptr)
{
	struct symbol *sym = ptr->ref.v;
	struct asf_stack_element *stack = sym->backend_status;
	str *result = asf_stack_get_element(stack, 0);
	if (result->s[result->len - 1] != '\0')
		str_append(result, 1, "\0");
	return result;
}

int asf_op_get_addr(struct expr *e)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	struct object_node *node = NULL;
	str *src = NULL;
	const char *temp = "lea%c %s, %%%s\n";
	if (e->valr->type.type != YZ_PTR)
		goto err_not_ptr;
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	dest = asf_reg_get(asf_yz_type2bytes(e->sum_type));
	if (*asf_regs[dest].purpose != ASF_REG_PURPOSE_NULL)
		if (asf_op_save_reg(node, dest))
			goto err_free_node;
	if ((src = op_ptr_get_addr_get_src(e->valr->v)) == NULL)
		goto err_free_node;
	if (src->s[src->len - 1] != '\0')
		str_append(src, 1, "\0");
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 3 + src->len);
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_regs[dest].bytes),
			src->s,
			asf_regs[dest].name);
	return 0;
err_not_ptr:
	printf("amc[backend.asf:%s]: asf_op_get_addr: "
			"Value is not a pointer!\n", __FILE__);
	return 1;
err_free_node:
	free(node);
	return 1;
}

struct asf_stack_element *asf_op_extract_get_mem(yz_extract_val *val)
{
	if (val->type == YZ_EXTRACT_ARRAY) {
		printf("amc[backend.asf]: asf_op_extract_get_mem: "
				"Unsupport action: 'YZ_EXTRACT_ARRAY'!\n");
		return NULL;
	} else if (val->type == YZ_EXTRACT_STRUCT) {
		return asf_struct_get_elem(val->sym->backend_status,
				val->index);
	}
	return NULL;
}

int asf_op_extract_ptr_val(struct symbol *sym)
{
	enum ASF_REGS dest = ASF_REG_RAX, src = ASF_REG_RAX;
	yz_ptr_type *ptr = NULL;
	if (op_ptr_extract_get_addr(&src, sym))
		return 1;
	ptr = sym->result_type.v;
	dest = asf_reg_get(asf_yz_type2bytes(&ptr->ref));
	return op_ptr_extract_from_reg(src, dest);
}

int asf_op_extract_ptr_val_from_expr(struct expr *expr)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	dest = asf_reg_get(asf_yz_type2bytes(expr->sum_type));
	return op_ptr_extract_from_reg(ASF_REG_RAX, dest);
}
