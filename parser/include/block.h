/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PARSER_BLOCK_H
#define AMC_PARSER_BLOCK_H
#include "../../include/file.h"
#include "../../include/symbol.h"

int block_check_start(struct file *f);
int parse_block(struct parser *parser);

#endif
