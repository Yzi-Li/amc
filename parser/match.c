#include "../include/token.h"
#include "block.h"
#include "keywords.h"

static int match_block_parse(struct file *f, struct symbol *fn);
static int match_condition_parse(struct file *f);
static int match_conditions_parse(struct file *f);

int match_block_parse(struct file *f, struct symbol *fn)
{
	return 0;
}

int match_condition_parse(struct file *f)
{
	return 0;
}

int match_conditions_parse(struct file *f)
{
	while (f->src[f->pos] != '\n') {
		match_condition_parse(f);
	}

	return 0;
}

int parse_match(struct file *f, struct symbol *t, struct symbol *fn)
{
	file_try_skip_space(f);

	match_conditions_parse(f);

	return 0;
}
