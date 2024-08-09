#include "keywords.h"
//#include "expr.h"

static int if_block_parse(struct file *f, struct symbol *fn);
static int if_condition_parse(struct file *f, struct symbol *fn);

int if_block_parse(struct file *f, struct symbol *fn)
{
	return 0;
}

int if_condition_parse(struct file *f, struct symbol *fn)
{
	//parse_expr(f, fn);
	return 0;
}

int parse_if(struct file *f, struct symbol *t, struct symbol *fn)
{
	while (f->src[f->pos] != '{') {
		if_condition_parse(f, fn);
	}

	if_block_parse(f, fn);

	return 0;
}

int parse_elif(struct file *f, struct symbol *t, struct symbol *fn)
{
	return 0;
}

int parse_else(struct file *f, struct symbol *t, struct symbol *fn)
{
	return 0;
}
