#ifndef AMC_BE_ASF_LABEL_H
#define AMC_BE_ASF_LABEL_H
#include "../../../include/backend/target.h"
#include "../../../utils/str/str.h"

typedef int label_id;

label_id asf_label_alloc();
label_id asf_label_get_last();
str *asf_label_get_str(label_id id);

#endif
