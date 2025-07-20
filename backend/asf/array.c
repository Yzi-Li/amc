#include "include/asf.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/op_val.h"
#include "include/stack.h"
#include "include/suffix.h"
#include "include/val.h"
#include "../../include/backend/object.h"
#include <stdlib.h>
#include <string.h>

static int array_elem_push(yz_val *val);
static int array_elem_push_empty(yz_val *type);
static str *array_get_elem_from_imm(struct asf_stack_element *base,
		struct asf_imm *src);
static str *array_get_elem_from_mem(struct asf_stack_element *base,
		struct asf_stack_element *src);
static str *array_get_elem_from_reg(struct asf_stack_element *base,
		enum ASF_REGS src);
static str *array_set_elem_from_imm(struct asf_stack_element *base,
		struct asf_imm *offset, str *src);
static str *array_set_elem_from_mem(struct asf_stack_element *base,
		struct asf_stack_element *mem, str *src);
static str *array_set_elem_from_reg(struct asf_stack_element *base,
		enum ASF_REGS reg, str *src);
static str *array_set_elem_get_val(yz_val *src);
static str *array_set_elem_get_val_reg(enum ASF_REGS reg);

int array_elem_push(yz_val *val)
{
	struct object_node *node = NULL;
	if (val == NULL)
		return array_elem_push_empty(val);
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_push(val)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: array_elem_push: "
			"Get instruction failed!\n",
			__FILE__);
err_free_node:
	free(node);
	return 1;
}

int array_elem_push_empty(yz_val *val)
{
	struct asf_imm imm = {
		.type = asf_yz_type2bytes(&val->type),
		.iq = 0
	};
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_push_imm(&imm)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: array_elem_push_empty: "
			"Get instruction failed!\n",
			__FILE__);
err_free_node:
	free(node);
	return 1;
}

str *array_get_elem_from_imm(struct asf_stack_element *base,
		struct asf_imm *src)
{
	enum ASF_REGS dest = asf_reg_get(base->bytes);
	int offset = 0;
	str *s = str_new();
	const char *temp = "mov%c -%lld(%%rbp), %%%s\n";
	offset = base->bytes * src->iq;
	str_expand(s, strlen(temp) - 4 + ullen(offset));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(base->bytes),
			base->addr - offset,
			asf_regs[dest].name);
	return s;
}

str *array_get_elem_from_mem(struct asf_stack_element *base,
		struct asf_stack_element *src)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	struct object_node *node = NULL;
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, src, &dest)) == NULL)
		goto err_free_node;
	return array_get_elem_from_reg(base, dest);
err_free_node:
	free(node);
	return NULL;
}

str *array_get_elem_from_reg(struct asf_stack_element *base, enum ASF_REGS src)
{
	enum ASF_REGS dest = asf_reg_get(base->bytes);
	str *s = str_new();
	const char *temp = "mov%c -%lld(%%rbp,%%%s,%lld), %%%s\n";
	str_expand(s, strlen(temp) - 8 + ullen(base->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(base->bytes),
			base->addr,
			asf_regs[src - asf_reg_get(asf_regs[src].bytes)].name,
			base->bytes,
			asf_regs[dest].name);
	return s;
}

str *array_set_elem_from_imm(struct asf_stack_element *base,
		struct asf_imm *offset, str *src)
{
	int dest = 0;
	str *s = str_new();
	const char *temp = "mov%c %s, -%lld(%%rbp)\n";
	dest = base->bytes * offset->iq;
	str_expand(s, strlen(temp) - 6 + src->len + ullen(dest));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(base->bytes),
			src->s,
			base->addr - dest);
	return s;
}

str *array_set_elem_from_mem(struct asf_stack_element *base,
		struct asf_stack_element *mem, str *src)
{
	enum ASF_REGS dest = ASF_REG_RAX;
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	if ((node->s = asf_inst_mov(ASF_MOV_M2R, mem, &dest)) == NULL)
		goto err_free_node;
	return array_set_elem_from_reg(base, dest, src);
err_free_node:
	free(node);
	return NULL;
}

str *array_set_elem_from_reg(struct asf_stack_element *base, enum ASF_REGS reg,
		str *src)
{
	str *s = str_new();
	const char *temp = "mov%c %s, -%lld(%%rbp,%%%s,%lld)\n";
	str_expand(s, strlen(temp) - 10
			+ src->len
			+ ullen(base->addr));
	snprintf(s->s, s->len, temp,
			asf_suffix_get(base->bytes),
			src->s,
			base->addr,
			asf_regs[reg - asf_reg_get(asf_regs[reg].bytes)].name,
			base->bytes);
	return s;
}

str *array_set_elem_get_val(yz_val *src)
{
	struct asf_val val = {};
	if (asf_val_get(src, &val))
		return NULL;
	if (val.type == ASF_VAL_IMM) {
		return asf_imm_str_new(&val.imm);
	} else if (val.type == ASF_VAL_REG) {
		return array_set_elem_get_val_reg(val.reg);
	} else if (val.type == ASF_VAL_MEM) {
		return asf_stack_get_element(val.mem, 0);
	}
	return NULL;
}

str *array_set_elem_get_val_reg(enum ASF_REGS reg)
{
	enum ASF_REGS dest = ASF_REG_RBX, tmp = ASF_REG_RAX;
	struct object_node *node = NULL;
	if (reg - asf_reg_get(asf_regs[reg].bytes) != tmp)
		return asf_reg_get_str(&asf_regs[reg]);
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	dest += asf_reg_get(asf_regs[reg].bytes);
	if ((node->s = asf_inst_mov(ASF_MOV_R2R, &reg, &dest)) == NULL)
		goto err_inst_failed;
	return asf_reg_get_str(&asf_regs[dest]);
err_inst_failed:
	printf("amc[backend.asf:%s]: array_set_elem_get_val_from_reg: "
			"Get instruction failed!\n", __FILE__);
err_free_node:
	free(node);
	return NULL;
}

int asf_array_def(backend_symbol_status **raw_sym_stat, yz_val **vs, int len)
{
	for (int i = len - 1; i != -1; i--) {
		if (array_elem_push(vs[i]))
			return 1;
	}
	if (asf_identifier_reg(raw_sym_stat, asf_stack_top))
		goto err_identifier_reg_failed;
	return 0;
err_identifier_reg_failed:
	printf("amc[backend.asf]: asf_array_def: "
			"Identifier register failed!\n");
	return 1;
}

int asf_array_set_elem(struct symbol *sym, yz_val *offset, yz_val *val,
		enum OP_ID mode)
{
	struct asf_val dest = {};
	struct object_node *node = NULL;
	str *src = NULL;
	if (asf_val_get(offset, &dest))
		return 1;
	if ((src = array_set_elem_get_val(val)) == NULL)
		return 1;
	if (src->s[src->len - 1] != '\0')
		str_append(src, 1, "\0");
	node = malloc(sizeof(*node));
	if (dest.type == ASF_VAL_IMM) {
		if ((node->s = array_set_elem_from_imm(sym->backend_status,
						&dest.imm, src)) == NULL)
			goto err_inst_failed;
	} else if (dest.type == ASF_VAL_MEM) {
		if ((node->s = array_set_elem_from_mem(sym->backend_status,
						dest.mem, src)) == NULL)
			goto err_inst_failed;
	} else if (dest.type == ASF_VAL_REG) {
		if ((node->s = array_set_elem_from_reg(sym->backend_status,
						dest.reg, src)) == NULL)
			goto err_inst_failed;
	}
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	str_free(src);
	return 0;
err_inst_failed:
	printf("amc[backend.asf]: asf_array_set_elem: "
			"Get instruction failed!\n");
	str_free(node->s);
err_free_node_and_str:
	free(node);
	str_free(src);
	return 1;
}

int asf_op_extract_array_elem(yz_extract_val *val)
{
	struct asf_stack_element *base = NULL;
	struct object_node *node = NULL;
	struct asf_val dest = {};
	if ((base = val->sym->backend_status) == NULL)
		goto err_identifier_not_found;
	if (asf_val_get(val->offset, &dest))
		return 1;
	node = malloc(sizeof(*node));
	if (dest.type == ASF_VAL_IMM) {
		if ((node->s = array_get_elem_from_imm(base, &dest.imm))
				== NULL)
			goto err_inst_failed;
	} else if (dest.type == ASF_VAL_MEM) {
		if ((node->s = array_get_elem_from_mem(base, dest.mem)) == NULL)
			goto err_inst_failed;
	} else if (dest.type == ASF_VAL_REG) {
		if ((node->s = array_get_elem_from_reg(base, dest.reg)) == NULL)
			goto err_inst_failed;
	}
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_identifier_not_found:
	printf("amc[backend.asf]: asf_array_get_elem: "
			"Identifier not found!\n");
	return 1;
err_inst_failed:
	printf("amc[backend.asf]: asf_array_get_elem: "
			"Get instruction failed!\n");
err_free_node:
	free(node);
	return 1;
err_free_node_and_str:
	str_free(node->s);
	goto err_free_node;
}
