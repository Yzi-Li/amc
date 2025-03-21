#ifndef AMC_BLOCK_H
#define AMC_BLOCK_H
#include "../include/file.h"
#include "../include/symbol.h"

int parse_block(int indent, struct file *f, struct scope *scope);

#endif
