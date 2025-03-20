#include "asf.h"
#include "inst.h"
#include "stack.h"
#include "suffix.h"
#include <stdio.h>
#include "../../include/backend/target.h"

enum ASF_STACK_MODE asf_stack_mode = ASF_STACK_MODE_NATIVE;
struct asf_stack_element *asf_stack_top = NULL;

static const char *temp_pop_local = "%d(%%rsp)";
static const char *temp_pop = "pop%c %%%s\n";
static const char *temp_push = "push%c %s\n";

static int stack_element_append(enum ASF_IMM_TYPE bytes);
static void stack_element_remove();
static int stack_mode_switch(enum ASF_STACK_MODE mode);

int stack_element_append(enum ASF_IMM_TYPE bytes)
{
	struct asf_stack_element *element = malloc(sizeof(*asf_stack_top));
	if (element == NULL)
		return 1;
	if (asf_stack_top == NULL) {
		asf_stack_top = element;
		asf_stack_top->next = NULL;
		asf_stack_top->prev = NULL;
		asf_stack_top->addr = 0;
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

int stack_mode_switch(enum ASF_STACK_MODE mode)
{
	str *s = NULL;
	const char *temp = "subq $%lld, %%rsp\n";
	if (asf_stack_mode == mode)
		return 0;
	s = str_new();
	str_expand(s, strlen(temp) - 1);
	snprintf(s->s, s->len, temp,
			asf_stack_top->addr + asf_stack_top->bytes);
	return 0;
}

str *asf_inst_pop(enum ASF_REGS *dest)
{
	str *s = NULL;
	s = str_new();
	if (*dest <= ASF_REG_RSP)
		*dest += asf_reg_get(asf_stack_top->bytes);
	str_expand(s, strlen(asf_regs[*dest].name)
			+ strlen(temp_pop) - 3);
	snprintf(s->s, s->len, temp_pop,
			asf_suffix_get(asf_stack_top->bytes),
			asf_regs[*dest].name);
	stack_element_remove();
	return s;
}

str *asf_inst_pop_local()
{
	str *s = str_new();
	const char *temp_rm_top = "subq $%lld, %%rsp\n";
	struct object_node *node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(objs[ASF_OBJ_TEXT], node))
		goto err_free_node;
	int top_bytes_len = ullen(asf_stack_top->bytes);
	str_expand(node->s, strlen(temp_rm_top) - 4 + top_bytes_len);
	str_expand(s, strlen(temp_pop_local) - 2 + top_bytes_len);
	snprintf(s->s, s->len, temp_pop_local, asf_stack_top->bytes);
	snprintf(node->s->s, node->s->len, temp_rm_top,
			asf_stack_top->bytes);
	stack_element_remove();
	return s;
err_free_node:
	str_free(node->s);
	free(node);
	return NULL;
}

str *asf_inst_push(enum ASF_IMM_TYPE bytes, const char *src,
		enum ASF_STACK_MODE mode)
{
	str *s = NULL;
	if (stack_mode_switch(mode))
		return NULL;
	s = str_new();
	str_expand(s, (strlen(temp_push) - 2) + strlen(src));
	snprintf(s->s, s->len, temp_push,
			asf_suffix_get(bytes),
			src);
	stack_element_append(bytes);
	return s;
}

str *asf_inst_pushi(struct asf_imm *imm, enum ASF_STACK_MODE mode)
{
	str *s = NULL;
	str *tmp = asf_imm_str_new(imm);
	s = asf_inst_push(imm->type, tmp->s, mode);
	str_free(tmp);
	return s;
}
