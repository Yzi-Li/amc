#include "include/decorator.h"
#include "../include/parser.h"
#include "../include/backend.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../utils/die.h"
#include "../utils/str/str.h"
#include "include/keywords.h"
#include <stdlib.h>

struct parser parser_global_conf = {0, 0, "/tmp/amc.target.s"};

static int parse_line(struct file *f, struct scope *scope,
		struct hooks **hooks);

int parse_line(struct file *f, struct scope *scope, struct hooks **hooks)
{
	char *err_msg;
	str token = TOKEN_NEW;
	struct symbol *sym = NULL;
	file_skip_space(f);
	if (parse_comment(f))
		return -1;
	if (f->src[f->pos] == '\n')
		return 0;
	if (f->src[f->pos] == '@')
		return parse_decorator(f, *hooks);
	if (token_next(&token, f))
		return 1;
	if (!keyword_find(&token, &sym))
		goto err_sym_not_found;
	if (!sym->flags.toplevel)
		goto err_not_toplevel;
	sym->hooks = *hooks;
	if (sym->parse_function(f, sym, scope))
		return 1;
	sym->hooks = NULL;
	*hooks = calloc(1, sizeof(**hooks));
	return 0;
err_sym_not_found:
	err_msg = str2chr(token.s, token.len);
	printf("amc: parser.parse_line: %lld,%lld: "
			"symbol not found from token\n"
			"| Token: \"%s\"\n",
			f->cur_line, f->cur_column, err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	free(err_msg);
	return 2;
err_not_toplevel:
	err_msg = str2chr(token.s, token.len);
	printf("amc: parser.parse_line: %lld,%lld: token is not toplevel.\n"
			"| Token: \"%s\"\n",
			f->cur_line, f->cur_column, err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 2;
}

int parser_init(const char *path, struct file *f)
{
	struct hooks *hooks = calloc(1, sizeof(*hooks));
	int ret = 0;
	struct scope toplevel = {
		.fn = NULL,
		.indent = 0,
		.parent = NULL,
		.status = NULL,
		.status_type = SCOPE_TOP,
		.sym_groups = {}
	};
	if (file_init(path, f))
		die("amc: file_init: no such file: %s\n", path);
	if (backend_file_new(f))
		die("amc: backend_file_new: cannot create new file");

	while (f->src[f->pos] != '\0') {
		if ((ret = parse_line(f, &toplevel, &hooks)) > 0)
			return 1;
		if (f->src[f->pos] == '\0')
			break;
		if (ret != -1)
			file_line_next(f);
	}
	return backend_end();
}
