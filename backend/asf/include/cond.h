#ifndef AMC_BE_ASF_COND_H
#define AMC_BE_ASF_COND_H

struct asf_cond_handle {
	struct object_node **branch;
	int branch_num;
};

int asf_cond_handle_end(struct asf_cond_handle *handle);

#endif
