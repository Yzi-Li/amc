/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/cmp.h"
#include "include/jmp.h"
#include "include/label.h"
#include "include/op.h"
#include "include/val.h"
#include "../../include/backend/object.h"
#include <stdlib.h>

struct cond_handle {
	struct object_node **branch;
	int branch_num;
	label_id exit_label;
};

static int cond_append_branch(struct cond_handle *handle,
		struct object_node *node);
static int cond_append_exit_label(label_id label);
static int cond_append_jmp_exit(struct object_node *branch, str *jmp);
static int cond_end_branches(struct cond_handle *handle, label_id label);
static str *cond_get_jmp_exit(label_id label);

static void free_cond_handle(struct cond_handle *handle);

int cond_append_branch(struct cond_handle *handle,
		struct object_node *node)
{
	handle->branch_num += 1;
	handle->branch = realloc(handle->branch,
			sizeof(*handle->branch) * handle->branch_num);
	handle->branch[handle->branch_num - 1] = node;
	return 0;
}

int cond_append_exit_label(label_id label)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = asf_label_get_str(label);
	node->s->len -= 1;
	str_append(node->s, 2, ":\n");
	return 0;
err_free_node:
	free(node);
	return 1;
}

int cond_append_jmp_exit(struct object_node *branch, str *jmp)
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_insert(node, branch->prev, branch))
		goto err_free_node;
	node->s = jmp;
	return 0;
err_free_node:
	free(node);
	return 1;
}

int cond_end_branches(struct cond_handle *handle, label_id label)
{
	str *jmp_temp = NULL, *jmp = NULL;
	if (handle->branch_num == 1)
		return 0;
	if ((jmp_temp = cond_get_jmp_exit(label)) == NULL)
		return 1;
	for (int i = 0; i < handle->branch_num; i++) {
		jmp = str_new();
		str_copy(jmp_temp, jmp);
		if (handle->branch[i] == NULL)
			return 1;
		if (cond_append_jmp_exit(handle->branch[i], jmp))
			return 1;
	}
	str_free(jmp_temp);
	return 0;
}

str *cond_get_jmp_exit(label_id label)
{
	str *label_str = asf_label_get_str(label),
	    *s = NULL;
	s = asf_inst_jmp(ASF_JMP_ALWAYS, label_str->s, label_str->len);
	if (s == NULL)
		goto err_inst_failed;
	str_free(label_str);
	return s;
err_inst_failed:
	str_free(label_str);
	return NULL;
}

void free_cond_handle(struct cond_handle *handle)
{
	struct cond_handle *c = handle;
	if (handle == NULL)
		return;
	if (c->branch)
		free(c->branch);
	free(c);
}

int asf_cond_elif(backend_cond_if_handle *handle)
{
	struct cond_handle *c = handle;
	if (c == NULL)
		return 1;
	if (cond_append_exit_label(c->exit_label))
		return 1;
	if (cond_append_branch(c, cur_obj->sections[ASF_OBJ_TEXT].last))
		return 1;
	return 0;
}

int asf_cond_else(backend_cond_if_handle *handle)
{
	struct cond_handle *c = handle;
	if (c == NULL)
		return 1;
	c->exit_label = asf_label_alloc();
	if (cond_append_exit_label(c->exit_label))
		return 1;
	return 0;
}

int asf_cond_if(backend_cond_if_handle *handle)
{
	struct cond_handle *c = handle;
	if (c == NULL)
		return 1;
	if (cond_append_exit_label(c->exit_label))
		return 1;
	if (cond_append_branch(c, cur_obj->sections[ASF_OBJ_TEXT].last))
		return 1;
	return 0;
}

backend_cond_if_handle *asf_cond_if_begin(void)
{
	struct cond_handle *result = calloc(1, sizeof(*result));
	return result;
}

int asf_cond_if_cond(backend_cond_if_handle *handle)
{
	struct cond_handle *c = handle;
	if (c == NULL)
		return 1;
	c->exit_label = asf_label_get_last();
	return 0;
}

int asf_cond_if_end(backend_cond_if_handle *handle)
{
	if (cond_end_branches(handle, asf_label_get_last()))
		return 1;
	free_cond_handle(handle);
	return 0;
}

void asf_cond_if_free_handle(backend_cond_if_handle *handle)
{
	free_cond_handle(handle);
}

backend_cond_match_handle *asf_cond_match_begin(yz_val *val)
{
	struct cond_handle *result = calloc(1, sizeof(*result));
	return result;
}

int asf_cond_match_case(backend_cond_match_handle *handle, yz_val *val)
{
	struct cond_handle *c = handle;
	str *jmp, *label;
	struct object_node *node;
	struct asf_val src, dest = {
		.data.reg = ASF_OP_RESULT_REG,
		.type = ASF_VAL_REG
	};
	if (c == NULL)
		return 1;
	if (asf_val_get(val, &src))
		return 1;
	dest.data.reg += asf_reg_get(asf_yz_type2bytes(&val->type));
	node = malloc(sizeof(*node));
	if ((node->s = asf_inst_cmp(&src, &dest)) == NULL)
		goto err_free_node;
	c->exit_label = asf_label_alloc();
	label = asf_label_get_str(c->exit_label);
	jmp = asf_inst_jmp(ASF_JMP_NE, label->s, label->len);
	str_free(label);
	str_append(node->s, jmp->len, jmp->s);
	str_free(jmp);
	if (object_append(&cur_obj->sections[ASF_OBJ_TEXT], node))
		goto err_free_node_and_str;
	return 0;
err_free_node_and_str:
	str_free(node->s);
err_free_node:
	free(node);
	return 1;
}

int asf_cond_match_case_end(backend_cond_match_handle *handle)
{
	struct cond_handle *c = handle;
	if (c == NULL)
		return 1;
	if (cond_append_exit_label(c->exit_label))
		return 1;
	if (cond_append_branch(handle, cur_obj->sections[ASF_OBJ_TEXT].last))
		return 1;
	return 0;
}

int asf_cond_match_end(backend_cond_match_handle *handle)
{
	if (cond_end_branches(handle, asf_label_get_last()))
		return 1;
	free_cond_handle(handle);
	return 0;
}

void asf_cond_match_free_handle(backend_cond_match_handle *handle)
{
	free_cond_handle(handle);
}
