#ifndef AMC_BE_ASF_SCOPE_H
#define AMC_BE_ASF_SCOPE_H
#include "cond.h"

struct asf_scope_status {
	enum {
		ASF_SCOPE_STATUS_NORMAL,
		ASF_SCOPE_STATUS_COND
	} type;
	struct asf_cond_handle cond;
	struct object_node *end_node;
};

#endif
