/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_COND_H
#define AMC_BE_ASF_COND_H
#include "label.h"

struct asf_cond_handle {
	struct object_node **branch;
	int branch_num;
	label_id exit_label;
};

int asf_cond_handle_end(struct asf_cond_handle *handle);

#endif
