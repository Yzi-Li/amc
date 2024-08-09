#ifndef AMC_IDENTIFIER_H
#define AMC_IDENTIFIER_H
#include "symbol.h"

int parse_immut_var(struct file *f, struct symbol *sym, struct symbol *fn);
int parse_mut_var(struct file *f, struct symbol *sym, struct symbol *fn);

#endif
