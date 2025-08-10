/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_LOOP_H
#define AMC_BE_ASF_LOOP_H
#include "label.h"

struct asf_loop_handle {
	label_id cond_end_label;
};

int asf_loop_handle_end(struct asf_loop_handle *handle);

#endif
