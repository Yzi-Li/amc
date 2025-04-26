#ifndef AMC_BE_ASF_LABEL_H
#define AMC_BE_ASF_LABEL_H
#include "../../../include/backend/target.h"
#include "../../../utils/str/str.h"

typedef int label_id;

int asf_block_append(struct object_node *node);
label_id asf_label_alloc();
str *asf_label_get_str(label_id id);

#endif
