#include "register.h"
#include "../../include/type.h"
#include "../../include/backend/target.h"

int asf_op_cmp(struct object_node *node,
		yz_val *l, yz_val *r);
int asf_op_save_reg(struct object_node *root, struct asf_reg *reg);
int asf_op_save_val(struct object_node *root, yz_val *v,
		enum ASF_REGS r);
