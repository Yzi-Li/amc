#include "include/ptr.h"
#include "include/type.h"
#include "../include/ptr.h"
#include <stdlib.h>

int parse_type_ptr(struct file *f, yz_val *ptr)
{
	yz_ptr *box = NULL;
	file_pos_next(f);
	file_skip_space(f);
	box = malloc(sizeof(yz_ptr));
	ptr->type = YZ_PTR;
	ptr->v = box;
	if (parse_type(f, &box->ref))
		return 1;
	if (f->src[f->pos] == '?') {
		file_pos_next(f);
		file_skip_space(f);
		return -1;
	}
	return 0;
}
