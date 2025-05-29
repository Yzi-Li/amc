#include "include/asf.h"
#include "include/identifier.h"
#include "include/mov.h"
#include "include/register.h"
#include "include/stack.h"
#include "../../include/backend/object.h"
#include "../../include/symbol.h"

static int op_assign_get_val(struct expr *e, int (*op_func)(struct expr *e));

int op_assign_get_val(struct expr *e, int (*op_func)(struct expr *e))
{
	struct asf_stack_element *dest = NULL;
	struct object_node *node = NULL;
	enum ASF_REGS src = ASF_REG_RAX;
	struct symbol *sym = e->vall->v;
	char *name = NULL;
	if (op_func(e))
		return 1;
	node = malloc(sizeof(*node));
	if (object_append(&objs[cur_obj][ASF_OBJ_TEXT], node))
		goto err_free_node;
	name = str2chr(sym->name, sym->name_len);
	dest = asf_identifier_get(name);
	free(name);
	src = asf_reg_get(dest->bytes);
	if ((node->s = asf_inst_mov(ASF_MOV_R2M, &src, dest)) == NULL)
		goto err_inst_failed;
	return 0;
err_inst_failed:
	printf("amc[backend.asf:%s]: op_assign_get_val: "
			"Get instruction failed!\n", __FILE__);
err_free_node:
	free(node);
	return 1;
}

int asf_op_assign(struct expr *e)
{
	char *name = NULL;
	struct symbol *sym = e->vall->v;
	name = str2chr(sym->name, sym->name_len);
	if (asf_var_set(name, e->valr))
		goto err_call_failed;
	free(name);
err_call_failed:
	free(name);
	return 1;
}

int asf_op_assign_add(struct expr *e)
{
	return op_assign_get_val(e, asf_op_add);
}

int asf_op_assign_div(struct expr *e)
{
	return op_assign_get_val(e, asf_op_div);
}

int asf_op_assign_mul(struct expr *e)
{
	return op_assign_get_val(e, asf_op_mul);
}

int asf_op_assign_sub(struct expr *e)
{
	return op_assign_get_val(e, asf_op_sub);
}
