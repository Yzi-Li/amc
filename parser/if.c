#include "keywords.h"
//#include "expr.h"

static int if_block_parse(struct file *f, struct scope *scope);
static int if_condition_parse(struct file *f, struct scope *scope);

int if_block_parse(struct file *f, struct scope *scope)
{
	return 0;
}

int if_condition_parse(struct file *f, struct scope *scope)
{
	//parse_expr(f, fn);
	return 0;
}

int parse_if(struct file *f, struct symbol *t, struct scope *scope)
{
	while (f->src[f->pos] != '{') {
		if_condition_parse(f, scope);
	}

	if_block_parse(f, scope);

	return 0;
}

int parse_elif(struct file *f, struct symbol *t, struct scope *scope)
{
	return 0;
}

int parse_else(struct file *f, struct symbol *t, struct scope *scope)
{
	return 0;
}
