#include "include/asf.h"
#include "include/jmp.h"
#include "include/label.h"
#include "../../include/backend/target.h"
#include "../../utils/str/str.h"
#include "../../utils/utils.h"
#include <stdio.h>

static const char *insts[] = {
	[ASF_JMP_EQ] = "je %s\n",
	[ASF_JMP_GE] = "jge %s\n",
	[ASF_JMP_GT] = "jg %s\n",
	[ASF_JMP_LE] = "jle %s\n",
	[ASF_JMP_LT] = "jl %s\n",
	[ASF_JMP_NE] = "jne %s\n"
};

int asf_inst_jmp(enum ASF_JMP_TYPE inst, const char *label, int label_len)
{
	const char *temp;
	struct object_node *node;
	if (inst > LENGTH(insts))
		return 1;
	temp = insts[inst];
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	node->s = str_new();
	str_expand(node->s, strlen(temp) - 1 + label_len);
	snprintf(node->s->s, node->s->len, temp, label);
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
