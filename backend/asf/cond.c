/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/asf.h"
#include "include/cmp.h"
#include "include/cond.h"
#include "include/jmp.h"
#include "include/label.h"
#include "include/op.h"
#include "include/scope.h"
#include "include/val.h"
#include "../../include/backend/object.h"
#include <stdlib.h>

static int cond_append_branch(struct asf_cond_handle *handle,
		struct object_node *node);
static int cond_append_exit_label(label_id label);
static int cond_append_jmp_exit(struct object_node *branch, str *jmp);
static int cond_end_branches(struct asf_cond_handle *handle, label_id label,
		int has_else);
static str *cond_get_jmp_exit(label_id label);

int cond_append_branch(struct asf_cond_handle *handle,
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

int cond_end_branches(struct asf_cond_handle *handle, label_id label,
		int has_else)
{
	str *jmp_temp = NULL, *jmp = NULL;
	if (!has_else && handle->branch_num == 1) {
		free(handle->branch);
		return 0;
	}
	if ((jmp_temp = cond_get_jmp_exit(label)) == NULL)
		goto err_free_branches;
	for (int i = 0; i < handle->branch_num; i++) {
		jmp = str_new();
		str_copy(jmp_temp, jmp);
		if (handle->branch[i] == NULL)
			return 1;
		if (cond_append_jmp_exit(handle->branch[i], jmp))
			return 1;
	}
	str_free(jmp_temp);
	handle->branch_num = 0;
	free(handle->branch);
	return 0;
err_free_branches:
	handle->branch_num = 0;
	free(handle->branch);
	return 1;
}

str *cond_get_jmp_exit(label_id label)
{
	str *label_str = asf_label_get_str(label),
	    *s = NULL;
	if ((s = asf_inst_jmp(ASF_JMP_ALWAYS, label_str->s,
					label_str->len)) == NULL)
		goto err_inst_failed;
	str_free(label_str);
	return s;
err_inst_failed:
	str_free(label_str);
	return NULL;
}

int asf_cond_elif(backend_scope_status *raw_status)
{
	struct asf_scope_status *status = raw_status;
	if (cond_append_exit_label(asf_label_get_last()))
		return 1;
	if (cond_append_branch(&status->data.cond,
				cur_obj->sections[ASF_OBJ_TEXT].last))
		return 1;
	return 0;
}

int asf_cond_else(backend_scope_status *raw_status)
{
	label_id label = asf_label_alloc();
	struct asf_scope_status *status = raw_status;
	if (cond_append_exit_label(label))
		return 1;
	status->type = ASF_SCOPE_STATUS_NO;
	if (cond_end_branches(&status->data.cond, label, 1))
		return 1;
	return 0;
}

int asf_cond_if(backend_scope_status *raw_status)
{
	struct asf_scope_status *status = raw_status;
	if (status->type != ASF_SCOPE_STATUS_COND)
		return 1;
	if (cond_append_exit_label(status->data.cond.exit_label))
		return 1;
	status->data.cond.branch = malloc(sizeof(*status->data.cond.branch));
	status->data.cond.branch[0] = cur_obj->sections[ASF_OBJ_TEXT].last;
	status->data.cond.branch_num = 1;
	return 0;
}

int asf_cond_if_begin(backend_scope_status *raw_status)
{
	struct asf_scope_status *status = raw_status;
	status->data.cond.exit_label = asf_label_get_last();
	if (status->type != ASF_SCOPE_STATUS_NO)
		if (asf_scope_end(raw_status))
			return 1;
	status->type = ASF_SCOPE_STATUS_COND;
	return 0;
}

backend_cond_match_handle *asf_cond_match_begin(void)
{
	struct asf_cond_handle *result = calloc(1, sizeof(*result));
	return result;
}

int asf_cond_match_case(backend_cond_match_handle *handle, yz_val *val)
{
	struct asf_cond_handle *c = handle;
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
	struct asf_cond_handle *c = handle;
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
	if (cond_end_branches(handle, asf_label_get_last(), 0))
		return 1;
	free(handle);
	return 0;
}

void asf_cond_match_free_handle(backend_cond_match_handle *handle)
{
	struct asf_cond_handle *h = handle;
	if (handle == NULL)
		return;
	if (h->branch)
		free(h->branch);
	free(h);
}

int asf_cond_handle_end(struct asf_cond_handle *handle)
{
	if (cond_end_branches(handle, asf_label_get_last(), 0))
		return 1;
	return 0;
}
