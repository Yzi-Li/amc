/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_IDENTIFIER_H
#define AMC_IDENTIFIER_H
#include "file.h"
#include "symbol.h"

int parse_immut_var(struct file *f, struct symbol *sym, struct scope *fn);
int parse_mut_var(struct file *f, struct symbol *sym, struct scope *fn);

#endif
