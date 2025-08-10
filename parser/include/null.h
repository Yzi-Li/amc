/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_PARSER_NULL_H
#define AMC_PARSER_NULL_H

#define CHR_IS_NULL(CHR) (strncmp(CHR_NULL, (CHR), strlen(CHR_NULL)) == 0)
static const char *CHR_NULL = "null";

#endif
