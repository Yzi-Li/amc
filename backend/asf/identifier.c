#include "include/asf.h"
#include "include/call.h"
#include "include/identifier.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "../../include/symbol.h"
#include "../../include/backend/object.h"

typedef int sym_id_t;

struct defined_sym {
	char *index;
	struct asf_stack_element *stack;
};

static struct defined_sym *defined_syms = NULL;
static int defined_sym_len = 0;
static int defined_sym_capacity = 0;

static sym_id_t get_id(char *name);
static sym_id_t reg_id(char *name);
static str *set_expr(sym_id_t id, struct expr *expr);
static str *set_identifier(sym_id_t id, struct symbol *sym);
static str *set_imm(sym_id_t id, yz_val *val);
static str *set_sym(sym_id_t id, struct symbol *sym);
static str *set_val(sym_id_t id, yz_val *val);

sym_id_t get_id(char *name)
{
	if (name == NULL)
		return -1;
	for (int i = 0; i < defined_sym_len; i++) {
		if (strcmp(name, defined_syms[i].index) == 0)
			return i;
	}
	return -1;
}

sym_id_t reg_id(char *name)
{
	if (name == NULL)
		return -1;
	if (defined_syms == NULL) {
		defined_sym_capacity = 1;
		defined_sym_len = 1;
		defined_syms = malloc(sizeof(*defined_syms));
		defined_syms[0].index = name;
		return 0;
	}
	defined_sym_len += 1;
	if (defined_sym_len > defined_sym_capacity) {
		defined_sym_capacity += 1;
		defined_syms = realloc(defined_syms,
				defined_sym_capacity * sizeof(*defined_syms));
	}
	defined_syms[defined_sym_len - 1].index = name;
	return defined_sym_len - 1;
}

str *set_expr(sym_id_t id, struct expr *expr)
{
	enum ASF_REGS src = asf_reg_get(asf_yz_type_raw2bytes(*expr->sum_type));
	return asf_inst_mov(ASF_MOV_R2M, &src, defined_syms[id].stack);
}

str *set_identifier(sym_id_t id, struct symbol *sym)
{
	struct asf_stack_element *src = NULL;
	sym_id_t src_id = -1;
	char *src_name = NULL;
	src_name = str2chr(sym->name, sym->name_len);
	if ((src_id = get_id(src_name)) == -1)
		goto err_id_not_exists;
	free(src_name);
	src = defined_syms[src_id].stack;
	return asf_inst_mov(ASF_MOV_M2M, src, defined_syms[id].stack);
err_id_not_exists:
	printf("amc[backend.asf:%s]: set_identifier: "
			"Identifier not exists!\n",
			__FILE__);
	free(src_name);
	return NULL;
}

str *set_imm(sym_id_t id, yz_val *val)
{
	struct asf_imm imm = {
		.type = asf_yz_type_raw2bytes(val->type),
		.iq = val->l
	};
	return asf_inst_mov(ASF_MOV_I2M, &imm, defined_syms[id].stack);
}

str *set_sym(sym_id_t id, struct symbol *sym)
{
	enum ASF_REGS src = asf_reg_get(asf_yz_type2bytes(&sym->result_type));
	if (sym->args == NULL && sym->argc == 1)
		return set_identifier(id, sym);
	if (sym->args == NULL && sym->argc > 1) {
		if (sym->argc - 2 > asf_call_arg_regs_len)
			return NULL;
		src += asf_call_arg_regs[sym->argc - 2];
	}
	return asf_inst_mov(ASF_MOV_R2M, &src, defined_syms[id].stack);
}

str *set_val(sym_id_t id, yz_val *val)
{
	if (val->type == AMC_EXPR) {
		return set_expr(id, val->v);
	} else if (val->type == AMC_SYM) {
		return set_sym(id, val->v);
	} else if (YZ_IS_DIGIT(val->type)) {
		return set_imm(id, val);
	}
	printf("amc[backend.asf]: asf_var_set: Unsupport type: \"%s\"\n",
				yz_get_type_name(val));
	return NULL;
}

int asf_var_set(char *name, yz_val *val)
{
	sym_id_t id = -1;
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((id = get_id(name)) == -1) {
		if ((id = reg_id(name)) == -1)
			goto err_free_node;
		if ((node->s = asf_inst_push(val)) == NULL)
			goto err_inst_failed;
		defined_syms[id].stack = asf_stack_top;
	} else {
		if ((node->s = set_val(id, val)) == NULL)
			goto err_inst_failed;
	}
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_var_set: Get instruction failed!\n");
	goto err_free_node;
err_free_node:
	free(node);
	return 1;
}

int asf_var_immut_init(char *name, yz_val *val)
{
	return asf_var_set(name, val);
}

void asf_identifier_free_id(int num)
{
	if (num == 0)
		num = defined_sym_len;
	if (defined_sym_len == 0)
		return;
	for (int i = 0; i < num; i++) {
		defined_sym_len--;
		if (defined_syms[defined_sym_len].index != NULL)
			free(defined_syms[defined_sym_len].index);
	}
}

struct asf_stack_element *asf_identifier_get(char *name)
{
	sym_id_t id = -1;
	if ((id = get_id(name)) == -1)
		return NULL;
	return defined_syms[id].stack;
}

int asf_identifier_reg(char *name, struct asf_stack_element *src)
{
	sym_id_t id = -1;
	if (name == NULL || src == NULL)
		return 1;
	if ((id = get_id(name)) != -1)
		goto err_registered;
	if ((id = reg_id(name)) == -1)
		return 1;
	defined_syms[id].stack = src;
	return 0;
err_registered:
	printf("amc[backend.asf]: asf_identifier_reg: "
			"Identifier registered!\n");
	return 1;
}
