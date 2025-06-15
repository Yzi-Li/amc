#include "include/bytes.h"
#include "include/call.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct asf_stack_element *asf_stack_top = NULL;

static int stack_element_append(enum ASF_BYTES bytes);
static void stack_element_remove();

int stack_element_append(enum ASF_BYTES bytes)
{
	struct asf_stack_element *element = malloc(sizeof(*asf_stack_top));
	if (element == NULL)
		return 1;
	if (asf_stack_top == NULL) {
		asf_stack_top = element;
		asf_stack_top->next = NULL;
		asf_stack_top->prev = NULL;
		asf_stack_top->addr = asf_bytes_get_size(bytes);
		asf_stack_top->bytes = bytes;
		return 0;
	}
	element->next = NULL;
	element->prev = asf_stack_top;
	element->addr = element->prev->addr + asf_bytes_get_size(bytes);
	element->bytes = bytes;
	asf_stack_top->next = element;
	asf_stack_top = element;
	return 0;
}

void stack_element_remove()
{
	struct asf_stack_element *element = asf_stack_top;
	if (asf_stack_top->prev == NULL) {
		free_cl(asf_stack_top);
		return;
	}
	asf_stack_top->prev->next = NULL;
	asf_stack_top = asf_stack_top->prev;
	free(element);
}

str *asf_inst_pop(enum ASF_REGS dest)
{
	str *s = NULL;
	s = asf_inst_mov(ASF_MOV_M2R, asf_stack_top, &dest);
	stack_element_remove();
	return s;
}

str *asf_inst_push(yz_val *val)
{
	struct asf_imm imm = {};
	if (val->type == AMC_EXPR) {
		return asf_inst_push_expr(val->v);
	} else if (val->type == AMC_SYM) {
		return asf_inst_push_sym(val->v);
	} else if (val->type == YZ_NULL) {
		imm.type = ASF_BYTES_U64;
		imm.iq = 0;
		return asf_inst_push_imm(&imm);
	} else if (val->type == YZ_ARRAY) {
		return asf_inst_push_arr(val->v);
	} else if (YZ_IS_DIGIT(val->type) || val->type == YZ_CHAR) {
		imm.type = asf_yz_type_raw2bytes(val->type);
		imm.iq = val->l;
		return asf_inst_push_imm(&imm);
	}
	printf("amc[backend.asf]: asf_inst_push: Unsupport type: '%s'!\n",
			yz_get_type_name(val));
	return NULL;
}

str *asf_inst_push_arr(yz_array *arr)
{
	if (stack_element_append(ASF_BYTES_U64))
		return NULL;
	if (arr->type.type != YZ_CHAR)
		return NULL;
	return asf_inst_mov(ASF_MOV_C2M, &arr->type.i, asf_stack_top);
}

str *asf_inst_push_expr(struct expr *expr)
{
	return asf_inst_push_reg(asf_reg_get(
				asf_yz_type_raw2bytes(*expr->sum_type)));
}

str *asf_inst_push_identifier(struct symbol *sym)
{
	struct asf_stack_element *src = sym->backend_status;
	return asf_inst_push_mem(src);
}

str *asf_inst_push_imm(struct asf_imm *imm)
{
	if (stack_element_append(imm->type))
		return NULL;
	return asf_inst_mov(ASF_MOV_I2M, imm, asf_stack_top);
}

str *asf_inst_push_mem(struct asf_stack_element *src)
{
	if (stack_element_append(src->bytes))
		return NULL;
	return asf_inst_mov(ASF_MOV_M2M, src, asf_stack_top);
}

str *asf_inst_push_reg(enum ASF_REGS src)
{
	if (stack_element_append(asf_regs[src].bytes))
		return NULL;
	*asf_regs[src].purpose = ASF_REG_PURPOSE_NULL;
	return asf_inst_mov(ASF_MOV_R2M, &src, asf_stack_top);
}

str *asf_inst_push_sym(struct symbol *sym)
{
	enum ASF_REGS src = ASF_REG_RAX;
	if (sym->args == NULL && sym->argc == 1)
		return asf_inst_push_identifier(sym);
	src = asf_reg_get(asf_yz_type2bytes(&sym->result_type));
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			return NULL;
		src += asf_call_arg_regs[sym->argc - 2];
	}
	return asf_inst_push_reg(src);
}

void asf_stack_end_frame(struct asf_stack_element *start)
{
	struct asf_stack_element *cur = NULL, *next = NULL;
	if (start == NULL) {
		cur = asf_stack_top;
	} else {
		cur = start->next;
		if (start->prev != NULL)
			start->prev->next = NULL;
	}
	asf_stack_top = start;
	while (cur != NULL) {
		next = cur->next;
		free(cur);
		cur = next;
	}
	return;
}

str *asf_stack_get_element(struct asf_stack_element *element, int pop)
{
	str *s = str_new();
	const char *temp = "-%d(%%rbp)";
	str_expand(s, strlen(temp) - 2 + ullen(element->addr));
	snprintf(s->s, s->len, temp, element->addr);
	if (pop)
		stack_element_remove();
	return s;
}
