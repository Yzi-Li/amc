#include "include/asf.h"
#include "include/identifier.h"
#include "include/imm.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "../../include/symbol.h"
#include "../../include/backend/target.h"

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
static int set_expr(char *name, struct expr *expr);
static int set_imm(char *name, yz_val *val);
static int set_sym(char *name, struct symbol *sym);

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

int set_expr(char *name, struct expr *expr)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	sym_id_t id = -1;
	struct object_node *node = calloc(1, sizeof(*node));
	dest = asf_reg_get(asf_yz_type2imm(*expr->sum_type));
	if ((id = get_id(name)) == -1) {
		if ((id = reg_id(name)) == -1)
			goto err_free_node;
		node->s = asf_inst_push_reg(dest);
		defined_syms[id].stack = asf_stack_top;
	} else {
		node->s = asf_inst_pop(dest);
	}
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int set_imm(char *name, yz_val *val)
{
	sym_id_t id = -1;
	struct asf_imm imm = {
		.type = asf_yz_type2imm(val->type),
		.iq = val->l
	};
	struct object_node *node = calloc(1, sizeof(*node));
	if ((id = get_id(name)) == -1) {
		if ((id = reg_id(name)) == -1)
			goto err_free_node;
		node->s = asf_inst_push_imm(&imm);
		defined_syms[id].stack = asf_stack_top;
	} else {
		node->s = asf_inst_mov(ASF_MOV_I2M, &imm,
				defined_syms[id].stack);
	}
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int set_sym(char *name, struct symbol *sym)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	sym_id_t id = -1;
	struct object_node *node = calloc(1, sizeof(*node));
	dest = asf_reg_get(asf_yz_type2imm(sym->result_type));
	if ((id = get_id(name)) == -1) {
		if ((id = reg_id(name)) == -1)
			goto err_free_node;
		node->s = asf_inst_push_reg(dest);
		defined_syms[id].stack = asf_stack_top;
	} else {
		node->s = asf_inst_pop(dest);
	}
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}

int asf_var_set(char *name, yz_val *val)
{
	if (val->type == AMC_EXPR) {
		return set_expr(name, val->v);
	} else if (val->type == AMC_SYM) {
		return set_sym(name, val->v);
	} else if (YZ_IS_DIGIT(val->type)) {
		return set_imm(name, val);
	}
	printf("amc[backend.asf]: asf_var_immut_init: Unsupport type: "
			"\"%s\"\n",
			yz_get_type_name(val->type));
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
