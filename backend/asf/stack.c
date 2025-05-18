#include "include/asf.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/suffix.h"
#include "../../include/backend/target.h"
#include <stdio.h>

struct asf_stack_element *asf_stack_top = NULL;

static int stack_element_append(enum ASF_IMM_TYPE bytes);
static void stack_element_remove();

int stack_element_append(enum ASF_IMM_TYPE bytes)
{
	struct asf_stack_element *element = malloc(sizeof(*asf_stack_top));
	if (element == NULL)
		return 1;
	if (asf_stack_top == NULL) {
		asf_stack_top = element;
		asf_stack_top->next = NULL;
		asf_stack_top->prev = NULL;
		asf_stack_top->addr = bytes;
		asf_stack_top->bytes = bytes;
		return 0;
	}
	element->next = NULL;
	element->prev = asf_stack_top;
	element->addr = element->prev->addr + bytes;
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
	if (stack_element_append(asf_regs[src].size))
		return NULL;
	*asf_regs[src].purpose = ASF_REG_PURPOSE_NULL;
	return asf_inst_mov(ASF_MOV_R2M, &src, asf_stack_top);
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
