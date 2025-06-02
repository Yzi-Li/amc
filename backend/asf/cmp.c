#include "include/asf.h"
#include "include/cmp.h"
#include "include/op.h"
#include "include/register.h"
#include "include/suffix.h"
#include "../../include/expr.h"
#include "../../utils/str/str.h"
#include <stdlib.h>
#include <string.h>

int asf_inst_cmp(struct expr *e)
{
	struct object_node *node = NULL;
	const char *temp = "cmp%c %s, %s\n";
	str *valr = NULL,
	    *vall = NULL;
	if ((vall = asf_op_get_val_right(NULL, e, -1)) == NULL)
		return 1;
	if ((valr = asf_op_get_val_left(NULL, e)) == NULL)
		return 1;
	node = malloc(sizeof(*node));
	node->s = str_new();
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	str_expand(node->s, strlen(temp) - 4
			+ vall->len
			+ valr->len);
	if (vall->s[valr->len - 1] != '\0')
		str_append(vall, 1, "\0");
	if (valr->s[valr->len - 1] != '\0')
		str_append(valr, 1, "\0");
	snprintf(node->s->s, node->s->len, temp,
			asf_suffix_get(asf_yz_type_raw2bytes(*e->sum_type)),
			vall->s,
			valr->s);
	*asf_regs[ASF_REG_RAX].purpose = ASF_REG_PURPOSE_NULL;
	return 0;
err_free_node:
	str_free(node->s);
	free(node);
	return 1;
}
