#include "include/indent.h"

int indent_read(struct file *f)
{
	int i = 0;
	while (f->src[f->pos] == '\t') {
		file_pos_next(f);
		i++;
	}
	return i;
}
