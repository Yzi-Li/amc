#include "../include/parser.h"
#include "../include/backend.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../utils/die.h"
#include "../utils/str/str.h"
#include "keywords.h"

struct parser parser_global_conf = {0, 0, 0};

static int parse_line(struct file *f, struct scope *scope);

int parse_line(struct file *f, struct scope *scope)
{
	char *err_msg;
	str token = TOKEN_NEW;
	struct symbol *sym = NULL;
	file_try_skip_space(f);
	if (parse_comment(f))
		return -1;
	if (f->src[f->pos] == '\n')
		return 0;
	if (token_next(&token, f))
		return 1;
	if (!keyword_find(&token, &sym))
		goto err_sym_not_found;
	if (!sym->flags.toplevel)
		goto err_not_toplevel;
	return sym->parse_function(f, sym, scope);
err_sym_not_found:
	err_msg = tok2str(token.s, token.len);
	printf("amc: parser.parse_line: symbol not found from token\n"
			"| Token: \"%s\"\n"
			"| In l:%lld,c:%lld\n",
			err_msg,
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	free(err_msg);
	return 2;
err_not_toplevel:
	printf("amc: parser.parse_line: token is not toplevel.\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 2;
}

int parser_init(const char *path, struct file *f)
{
	int ret = 0;
	struct scope toplevel = {
		.fn = NULL,
		.parent = NULL,
		.sym_groups = {}
	};
	if (file_init(path, f))
		die("amc: file_init: no such file: %s\n", path);
	if (backend_file_new(f))
		die("amc: backend_file_new: cannot create new file");

	while (f->src[f->pos] != '\0') {
		if ((ret = parse_line(f, &toplevel)) > 0)
			return 1;
		if (ret != -1)
			file_line_next(f);
	}
	return backend_end();
}
