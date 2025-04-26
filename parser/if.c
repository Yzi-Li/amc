#include "block.h"
#include "expr.h"
#include "keywords.h"
#include "../include/backend.h"
#include "../include/backend/block.h"
#include "../utils/die.h"

static int if_block_parse(struct file *f, struct symbol *sym,
		struct scope *scope);
static int if_condition_parse(struct file *f, struct symbol *sym,
		struct scope *scope);

int if_block_parse(struct file *f, struct symbol *sym, struct scope *scope)
{
	if (parse_block(1, f, scope))
		return 1;
	if (backend_call(block_end)())
		return 1;
	return 0;
}

int if_condition_parse(struct file *f, struct symbol *sym, struct scope *scope)
{
	struct expr *expr = NULL;
	if ((expr = parse_expr(f, 1, scope)) == NULL)
		goto err_cannot_parse_expr;
	if (expr_apply(expr) > 0)
		goto err_cannot_apply_expr;
	if (f->src[f->pos] == '\n')
		return file_line_next(f);
	return 0;
err_cannot_parse_expr:
	printf("|< amc: if_condition_parse: Cannot parse expression!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_cannot_apply_expr:
	printf("|< amc: if_condition_parse: Cannot apply expression!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int parse_if(struct file *f, struct symbol *sym, struct scope *scope)
{
	if (if_condition_parse(f, sym, scope))
		return 1;
	if (if_block_parse(f, sym, scope))
		return 1;
	return 0;
}

int parse_elif(struct file *f, struct symbol *sym, struct scope *scope)
{
	return 0;
}

int parse_else(struct file *f, struct symbol *sym, struct scope *scope)
{
	return 0;
}
