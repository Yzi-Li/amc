/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/identifier.h"
#include "include/ptr.h"
#include "include/type.h"
#include "include/utils.h"
#include "../include/backend.h"
#include "../include/parser.h"
#include "../include/ptr.h"
#include "../include/token.h"
#include <stdlib.h>

static int parse_type_ptr_sub(yz_type *result);

int parse_type_ptr_sub(yz_type *result)
{
	yz_ptr_type *root = result->v,
	            *box = root->ref.v;
	root->level += box->level;
	root->ref.type = box->ref.type;
	root->ref.v = box->ref.v;
	box->ref.type = AMC_ERR_TYPE;
	box->ref.v = NULL;
	free(box);
	return 0;
}

int parse_type_ptr(struct parser *parser, yz_type *result)
{
	str mut_str = {.len = 3, .s = "mut"};
	yz_ptr_type *box = NULL;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	box = malloc(sizeof(yz_ptr_type));
	box->level = 1;
	if (token_try_read(&mut_str, parser->f))
		box->flag_mut = 1;
	result->type = YZ_PTR;
	result->v = box;
	if (parse_type(parser, &box->ref))
		return 1;
	if (box->ref.type == YZ_PTR && parse_type_ptr_sub(result))
		return 1;
	if (parser->f->src[parser->f->pos] == '?') {
		file_pos_next(parser->f);
		file_skip_space(parser->f);
		box->flag_can_null = 1;
	}
	return 0;
}

int ptr_set_val(struct parser *parser, struct symbol *ident, enum OP_ID mode)
{
	yz_val *val = NULL;
	yz_ptr_type *ptr = ident->result_type.v;
	if (ident->result_type.type != YZ_PTR)
		return err_print_pos(__func__, NULL,
				parser->f->cur_line, parser->f->cur_column);
	if (identifier_assign_get_val(parser, &ptr->ref, &val))
		return 1;
	if (backend_call(ptr_set_val)(ident, val, mode))
		return err_print_pos(__func__, "Backend call failed!",
				parser->f->cur_line, parser->f->cur_column);
	free_yz_val(val);
	return 0;
}
