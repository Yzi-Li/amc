#include "include/decorator.h"
#include "../include/parser.h"
#include "../include/backend.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../utils/die.h"
#include "../utils/str/str.h"
#include "include/keywords.h"
#include <stdlib.h>

struct global_parser global_parser = { 0, 0, 0, "/tmp/amc.target.s" };

static int parse_line(struct parser *parser);

int parse_line(struct parser *parser)
{
	char *err_msg;
	str token = TOKEN_NEW;
	struct symbol *sym = NULL;
	file_skip_space(parser->f);
	if (parse_comment(parser->f))
		return -1;
	if (parser->f->src[parser->f->pos] == '\n')
		return 0;
	if (parser->f->src[parser->f->pos] == '@')
		return parse_decorator(parser->f, parser->hooks);
	if (token_next(&token, parser->f))
		return 1;
	if (!keyword_find(&token, &sym))
		goto err_sym_not_found;
	if (!sym->flags.toplevel)
		goto err_not_toplevel;
	parser->sym = sym;
	if (sym->parse_function(parser))
		return 1;
	return 0;
err_sym_not_found:
	err_msg = str2chr(token.s, token.len);
	printf("amc: parser.parse_line: %lld,%lld: "
			"symbol not found from token\n"
			"| Token: \"%s\"\n",
			parser->f->cur_line, parser->f->cur_column, err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	free(err_msg);
	return 2;
err_not_toplevel:
	err_msg = str2chr(token.s, token.len);
	printf("amc: parser.parse_line: %lld,%lld: token is not toplevel.\n"
			"| Token: \"%s\"\n",
			parser->f->cur_line, parser->f->cur_column, err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 2;
}

int parse_file(const char *path, struct file *f)
{
	struct scope toplevel = {
		.fn = NULL,
		.indent = 0,
		.parent = NULL,
		.status = NULL,
		.status_type = SCOPE_TOP,
		.sym_groups = {}
	};
	struct parser parser = {
		.f = f,
		.hooks = calloc(1, sizeof(*parser.hooks)),
		.scope = &toplevel,
	};
	int ret = 0;
	if (file_init(path, f))
		die("amc: file_init: no such file: %s\n", path);
	if (backend_file_new(f))
		die("amc: backend_file_new: cannot create new file: %s", path);

	while (f->src[f->pos] != '\0') {
		if ((ret = parse_line(&parser)) > 0)
			return 1;
		if (f->src[f->pos] == '\0')
			break;
		if (ret != -1)
			file_line_next(f);
	}
	return backend_file_end(global_parser.target_path);
}

int parser_init(const char *path, struct file *f)
{
	if (parse_file(path, f))
		return 1;
	return backend_end();
}
