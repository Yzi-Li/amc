#include "include/array.h"
#include "include/identifier.h"
#include "include/keywords.h"
#include "include/struct.h"
#include "include/type.h"
#include "../include/backend.h"
#include "../include/parser.h"

static int let_init_constructor(struct parser *parser, struct symbol *sym);
static int let_init_val(struct parser *parser, struct symbol *sym);
static int let_reg_sym(struct parser *parser, struct symbol *sym);

int let_init_constructor(struct parser *parser, struct symbol *sym)
{
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	if (parser->f->src[parser->f->pos] == '\n')
		file_line_next(parser->f);
	switch (sym->result_type.type) {
	case YZ_ARRAY:
		return constructor_array(parser, sym);
		break;
	case YZ_STRUCT:
		return constructor_struct(parser, sym);
		break;
	default:
		return 1;
		break;
	}
	return 0;
}

int let_init_val(struct parser *parser, struct symbol *sym)
{
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	if (parser->f->src[parser->f->pos] == '{')
		return let_init_constructor(parser, sym);
	if (identifier_assign_val(parser, sym, OP_ASSIGN))
		return 1;
	return keyword_end(parser->f);
}

int let_reg_sym(struct parser *parser, struct symbol *sym)
{
	sym->argc = 1;
	if (symbol_register(sym, &parser->scope->sym_groups[SYMG_SYM]))
		goto err_cannot_register_sym;
	return 0;
err_cannot_register_sym:
	printf("amc: let_reg_sym: %lld,%lld: Cannot register symbol!\n",
			parser->f->cur_line, parser->f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_let(struct parser *parser)
{
	struct symbol *result = calloc(1, sizeof(*result));
	int ret=  0;
	result->type = SYM_IDENTIFIER;
	result->flags.mut = identifier_check_mut(parser->f);
	if ((ret = parse_type_name_pair(parser, &result->name,
					&result->result_type)) > 0)
		goto err_free_result;
	if (ret == -1)
		result->flags.can_null = 1;
	if (let_reg_sym(parser, result))
		goto err_free_result;
	if (parser->f->src[parser->f->pos] == '\n')
		return file_line_next(parser->f);
	if (parse_comment(parser->f))
		return 0;
	if (parser->f->src[parser->f->pos] != '=')
		goto err_syntax_err;
	return let_init_val(parser, result);
err_syntax_err:
	printf("amc: parse_let: %lld,%lld: Syntax error!\n",
			parser->f->cur_line, parser->f->cur_column);
err_free_result:
	free_symbol(result);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}
