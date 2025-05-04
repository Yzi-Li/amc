#include "include/asf.h"
#include "include/cond.h"
#include "include/jmp.h"

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
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
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
	str *jmp = NULL; // don't free
	if (!has_else && handle->branch_num == 1)
		return 0;
	if ((jmp = cond_get_jmp_exit(label)) == NULL)
		goto err_free_branches;
	for (int i = 0; i < handle->branch_num; i++) {
		if (handle->branch[i] == NULL)
			return 1;
		if (cond_append_jmp_exit(handle->branch[i], jmp))
			return 1;
	}
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
	free(label_str);
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
	if (cond_append_branch(&status->cond,
				objs[cur_obj][ASF_OBJ_TEXT].last))
		return 1;
	return 0;
}

int asf_cond_else(backend_scope_status *raw_status)
{
	label_id label = asf_label_alloc();
	struct asf_scope_status *status = raw_status;
	if (cond_append_exit_label(label))
		return 1;
	status->type = ASF_SCOPE_STATUS_NORMAL;
	if (cond_end_branches(&status->cond, label, 1))
		return 1;
	return 0;
}

int asf_cond_if(backend_scope_status *raw_status)
{
	struct asf_scope_status *status = raw_status;
	if (cond_append_exit_label(asf_label_get_last()))
		return 1;
	if (status->type == ASF_SCOPE_STATUS_COND) {
		if (asf_cond_handle_end(&status->cond))
			return 1;
	}
	status->type = ASF_SCOPE_STATUS_COND;
	status->cond.branch = malloc(sizeof(*status->cond.branch));
	status->cond.branch[0] = objs[cur_obj][ASF_OBJ_TEXT].last;
	status->cond.branch_num = 1;
	return 0;
}

int asf_cond_handle_end(struct asf_cond_handle *handle)
{
	if (cond_end_branches(handle, asf_label_get_last(), 0))
		return 1;
	return 0;
}
