#include "include/asf.h"
#include "include/call.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/val.h"
#include "../../include/backend/object.h"
#include "../../utils/utils.h"
#include <stdlib.h>
#include <string.h>

static int func_call_set_stack_top(int reverse);
static str *func_ret_imm(struct asf_imm *src);
static int func_ret_main(yz_val *v);
static str *func_ret_mem(struct asf_stack_element *src);
static str *func_ret_reg(enum ASF_REGS src);
static int func_ret_val(yz_val *v);

int func_call_set_stack_top(int reverse)
{
	struct object_node *node = malloc(sizeof(*node));
	const char *temp_normal = "subq $%lld, %%rsp\n",
	           *temp_reverse = "addq $%lld, %%rsp\n",
	           *temp = reverse ? temp_reverse : temp_normal;
	if (object_append(&cur_obj[ASF_OBJ_TEXT], node))
		return 1;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 4
			+ ullen(asf_stack_top->addr));
	snprintf(node->s->s, node->s->len, temp, asf_stack_top->addr);
	return 0;
}

str *func_ret_imm(struct asf_imm *src)
{
	enum ASF_REGS dest = asf_reg_get(src->type);
	return asf_inst_mov(ASF_MOV_I2R, src, &dest);
}

int func_ret_main(yz_val *v)
{
	struct object_node *node = malloc(sizeof(*node));
	if ((node->s = asf_inst_syscall(60, 1, &v)) == NULL)
		goto err_inst_failed;
	if (object_append(&cur_obj[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: func_ret_main: Get instruction failed!\n",
			__FILE__);
	free(node);
err_free_node_and_str:
	str_free(node->s);
	free(node);
	return 1;
}

str *func_ret_mem(struct asf_stack_element *src)
{
	enum ASF_REGS dest = asf_reg_get(src->bytes);
	return asf_inst_mov(ASF_MOV_M2R, src, &dest);
}

str *func_ret_reg(enum ASF_REGS src)
{
	enum ASF_REGS dest = asf_reg_get(asf_regs[src].bytes);
	*asf_regs[dest].purpose = ASF_REG_PURPOSE_NULL;
	*asf_regs[src].purpose = ASF_REG_PURPOSE_NULL;
	if (src == dest)
		return str_new();
	return asf_inst_mov(ASF_MOV_R2R, &src, &dest);
}

int func_ret_val(yz_val *v)
{
	struct object_node *node = NULL;
	struct asf_val val = {};
	if (asf_val_get(v, &val))
		return 1;
	node = malloc(sizeof(*node));
	if (val.type == ASF_VAL_IMM) {
		if ((node->s = func_ret_imm(&val.imm)) == NULL)
			goto err_inst_failed;
	} else if (val.type == ASF_VAL_MEM) {
		if ((node->s = func_ret_mem(val.mem)) == NULL)
			goto err_inst_failed;
	} else if (val.type == ASF_VAL_REG) {
		if ((node->s = func_ret_reg(val.reg)) == NULL)
			goto err_inst_failed;
	} else {
		return 1;
	}
	if (object_append(&cur_obj[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: func_ret_val: Get instruction failed!\n",
			__FILE__);
	free(node);
	return 1;
err_free_node_and_str:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_func_call(const char *name, yz_val *type, yz_val **vs, int vlen)
{
	struct object_node *node = NULL;
	enum ASF_REGS reg = ASF_REG_RAX;
	const char *temp = "call %s\n";
	if (vlen > asf_call_arg_regs_len)
		goto err_too_many_arg;
	reg = asf_reg_get(asf_yz_type2bytes(type));
	*asf_regs[reg].purpose = ASF_REG_PURPOSE_FUNC_RESULT;
	if (asf_call_push_args(vlen, vs))
		goto err_free_node;
	if (asf_stack_top != NULL && func_call_set_stack_top(0))
			return 1;
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if (asf_stack_top != NULL && func_call_set_stack_top(1))
		goto err_free_node;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 1 + strlen(name));
	snprintf(node->s->s, node->s->len, temp, name);
	return 0;
err_too_many_arg:
	printf("amc[backend.asf]: Too many arguments!\n");
	return 1;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_func_def(const char *name, int len, yz_val *type)
{
	const char *temp =
		".globl %s\n"
		"%s:\n"
		"pushq %%rbp\n"
		"movq %%rsp, %%rbp\n";
	char *tmp_name = malloc(len + 1);
	struct object_node *node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&cur_obj[ASF_OBJ_TEXT], node))
		goto err_free_node;
	memcpy(tmp_name, name, len);
	tmp_name[len] = '\0';
	str_expand(node->s, strlen(temp) - 6 + (len * 2));
	snprintf(node->s->s, node->s->len, temp, tmp_name, tmp_name);
	free(tmp_name);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_func_ret(yz_val *v, int is_main)
{
	const char *temp =
		"popq %rbp\n"
		"ret\n";
	struct object_node *node = NULL;
	if (is_main) {
		if (func_ret_main(v))
			return 1;
	} else if (func_ret_val(v)) {
		return 1;
	}
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	str_append(node->s, strlen(temp), temp);
	return 0;
err_free_node:
	free(node);
	return 1;
}

int asf_syscall(int code)
{
	const char *temp = "movq $%d, %%rax\nsyscall\n";
	struct object_node *tmp = NULL, *prev = NULL,
	                   *node = malloc(sizeof(*node));
	if (object_append(&cur_obj[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 2 + ullen(code));
	snprintf(node->s->s, node->s->len, temp, code);
	prev = asf_stack_top != NULL
		? node->prev->prev->prev
		: node->prev->prev;
	tmp = prev->next;
	prev->next = node;
	node->prev = prev;
	str_free(tmp->s);
	free(tmp);
	return 0;
err_free_node:
	free(node);
	return 1;
}
