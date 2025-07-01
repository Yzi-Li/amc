#ifndef AMC_BE_TARGET_H
#define AMC_BE_TARGET_H
#include "object.h"

int target_write(const char *target_path, struct object_head *obj, int len);

#endif
