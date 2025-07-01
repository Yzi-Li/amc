#include "include/ptr.h"
#include "include/type.h"
#include "../include/parser.h"
#include "../include/ptr.h"
#include <stdlib.h>

int parse_type_ptr(struct parser *parser, yz_val *ptr)
{
	yz_ptr *box = NULL;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	box = malloc(sizeof(yz_ptr));
	ptr->type = YZ_PTR;
	ptr->v = box;
	if (parse_type(parser, &box->ref))
		return 1;
	if (parser->f->src[parser->f->pos] == '?') {
		file_pos_next(parser->f);
		file_skip_space(parser->f);
		return -1;
	}
	return 0;
}
