/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_BE_ASF_LABEL_H
#define AMC_BE_ASF_LABEL_H
#include "../../../utils/str/str.h"

typedef int label_id;

label_id asf_label_alloc(void);
label_id asf_label_get_last(void);
str *asf_label_get_str(label_id id);

#endif
