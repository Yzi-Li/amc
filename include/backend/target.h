#ifndef AMC_BE_TARGET_H
#define AMC_BE_TARGET_H
#include "object.h"

int target_write(const char *path, int path_len, struct object_head *obj);

#endif
