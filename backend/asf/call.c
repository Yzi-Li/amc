#include "include/asf.h"
#include "include/call.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "include/val.h"
#include "../../include/backend/object.h"
#include <stdio.h>
#include <string.h>

static int call_push_arg(yz_val *v, int index);
static str *call_push_arg_const(int src, int index);
static str *call_push_arg_imm(struct asf_imm *src, int index);
static str *call_push_arg_mem(struct asf_stack_element *src, int index);
static str *call_push_arg_reg(enum ASF_REGS src, int index);

int call_push_arg(yz_val *v, int index)
{
	struct object_node *node = NULL;
	struct asf_val val = {};
	if (asf_val_get(v, &val))
		goto err_unsupport_type;
	node = malloc(sizeof(*node));
	if (val.type == ASF_VAL_CONST) {
		if ((node->s = call_push_arg_const(val.const_id, index))
				== NULL)
			goto err_inst_failed;
	} else if (val.type == ASF_VAL_IMM) {
		if ((node->s = call_push_arg_imm(&val.imm, index)) == NULL)
			goto err_inst_failed;
	} else if (val.type == ASF_VAL_MEM) {
		if ((node->s = call_push_arg_mem(val.mem, index)) == NULL)
			goto err_inst_failed;
	} else if (val.type == ASF_VAL_REG) {
		if ((node->s = call_push_arg_reg(val.reg, index)) == NULL)
			goto err_inst_failed;
	} else {
		goto err_unsupport_type;
	}
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_unsupport_type:
	printf("amc[backend.asf]: call_push_arg: "
			"Unsupport argument type: \"%s\"\n",
			yz_get_type_name(v));
	return 1;
err_inst_failed:
	printf("amc[backend.asf]: call_push_arg: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
err_free_node_and_str:
	str_free(node->s);
	goto err_free_node;
}

str *call_push_arg_const(int src, int index)
{
	enum ASF_REGS dest = asf_call_arg_regs[index]
		+ asf_reg_get(ASF_BYTES_U64);
	return asf_inst_mov(ASF_MOV_C2R, &src, &dest);
}

str *call_push_arg_imm(struct asf_imm *src, int index)
{
	enum ASF_REGS dest = asf_call_arg_regs[index]
		+ asf_reg_get(src->type);
	return asf_inst_mov(ASF_MOV_I2R, src, &dest);
}

str *call_push_arg_mem(struct asf_stack_element *src, int index)
{
	enum ASF_REGS dest = asf_call_arg_regs[index]
		+ asf_reg_get(src->bytes);
	return asf_inst_mov(ASF_MOV_M2R, src, &dest);
}

str *call_push_arg_reg(enum ASF_REGS src, int index)
{
	enum ASF_REGS dest = asf_call_arg_regs[index]
		+ asf_reg_get(asf_regs[src].bytes);
	return asf_inst_mov(ASF_MOV_R2R, &src, &dest);
}

int asf_call_push_args(int vlen, yz_val **vs)
{
	for (int i = 0; i < vlen; i++) {
		if (i > asf_call_arg_regs_len)
			goto err_too_many_arg;
		if (vs[i] == NULL)
			goto err_arg_not_exists;
		if (call_push_arg(vs[i], i))
			return 1;
	}
	return 0;
err_too_many_arg:
	printf("amc[backend.asf]: call_push_args: Too many arguments!\n");
	return 1;
err_arg_not_exists:
	printf("amc[backend.asf]: call_push_args: Argument is not exists!\n");
	return 1;
}

str *asf_inst_syscall(int code, int vlen, yz_val **vs)
{
	int str_last = 0;
	const char *temp =
		"movq $%d, %%rax\n"
		"syscall\n";
	str *s = str_new();
	if (asf_call_push_args(vlen, vs))
		goto err_free_str;
	str_last = s->len;
	str_expand(s, strlen(temp) - 2 + ullen(code));
	snprintf(&s->s[str_last], s->len, temp, code);
	return s;
err_free_str:
	str_free(s);
	return NULL;
}
