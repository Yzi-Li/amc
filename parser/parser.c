#include "include/cache.h"
#include "include/decorator.h"
#include "../include/parser.h"
#include "../include/backend.h"
#include "../include/symbol.h"
#include "../include/token.h"
#include "../utils/die.h"
#include "../utils/str/str.h"
#include "include/keywords.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__)
#include <libgen.h>
#elif defined(__WIN32)
#include <direct.h>
#endif

struct global_parser global_parser = {
	.has_err = 0,
	.has_main = 0,
	.parsed = {},
	.root_dir = {},
	.root_mod = {},
	.target_path = {}
};

static int parse_line(struct parser *parser);
static int parser_create_get_path(str *result, str *path);

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

int parser_create_get_path(str *result, str *path)
{
	if (path->len == global_parser.root_mod.len
			&& strncmp(path->s, global_parser.root_mod.s,
				global_parser.root_mod.len) == 0) {
		str_append(result, global_parser.root_mod.len,
				global_parser.root_mod.s);
		return 0;
	}
	str_expand(result, global_parser.root_mod.len + path->len + 2);
	snprintf(result->s, result->len, "%s/%s", global_parser.root_mod.s,
			path->s);
	result->len -= 1;
	return 0;
}

struct parser *parse_file(str *path, const char *real_path, struct file *f)
{
	struct parser *parser = parser_create(path, real_path, f);
	int ret = 0;
	if (file_init(real_path, f))
		die("amc: file_init: no such file: %s\n", path);
	if (backend_file_new(f))
		die("amc: backend_file_new: cannot create new file: %s", path);

	while (f->src[f->pos] != '\0') {
		if ((ret = parse_line(parser)) > 0)
			goto err_free_parser;
		if (f->src[f->pos] == '\0')
			break;
		if (ret != -1)
			file_line_next(f);
	}
	if (backend_file_end(parser->target.s, parser->target.len))
		goto err_free_parser;
	return parser;
err_free_parser:
	return NULL;
}

struct parser *parser_create(str *path, const char *real_path, struct file *f)
{
	struct parser *result = calloc(1, sizeof(*result));
	result->f = f;
	result->hooks = calloc(1, sizeof(*result->hooks));
	result->scope = calloc(1, sizeof(*result->scope));
	result->scope->fn = NULL;
	result->scope->indent = 0;
	result->scope->parent = NULL;
	result->scope->status = NULL;
	result->scope->status_type = SCOPE_TOP;
	if (parser_get_target_from_mod_path(&result->target, path))
		goto err_free_parser;
	if (parser_create_get_path(&result->path, path))
		goto err_free_parser;
	return result;
err_free_parser:
	free_parser(result);
	return NULL;
}

int parser_get_target_from_mod_path(str *result, str *path)
{
	str suffix = {};
	int suffix_need_free = 0;
	if ((suffix.s = backend_file_get_suffix(&result->len,
					&suffix_need_free)) == NULL)
		return 1;
	str_expand(result, global_parser.target_path.len
			+ path->len
			+ suffix.len
			+ 5);
	snprintf(result->s, result->len, "%s/%s.yz%s",
			global_parser.target_path.s,
			path->s, suffix.s);
	if (suffix_need_free)
		free(suffix.s);
	return 0;
}

int parser_init(const char *path, struct file *f)
{
	str path_cpy = {
		.len = strlen(path),
		.s = malloc(path_cpy.len + 1)
	};
	char *root_dir_cpy;
	strncpy(path_cpy.s, path, path_cpy.len);
	if (global_parser.root_dir.s == NULL) {
		global_parser.root_dir.s = dirname(path_cpy.s);
		global_parser.root_dir.len = strlen(global_parser.root_dir.s);
	}
	if (global_parser.root_mod.s == NULL) {
		root_dir_cpy = malloc(global_parser.root_dir.len + 1);
		strncpy(root_dir_cpy, global_parser.root_dir.s,
				global_parser.root_dir.len);
		global_parser.root_mod.s = basename(root_dir_cpy);
		global_parser.root_mod.len = strlen(global_parser.root_mod.s);
	}
	if (global_parser.target_path.s == NULL)
		if (cache_dir_create(&global_parser.root_dir))
			return 0;
	if (parse_file(&global_parser.root_mod, path, f) == NULL)
		return 1;
	return backend_end();
}

int parser_imported_append(struct parser_imported *imported, yz_module *mod)
{
	struct parser_imported_node *cur =
		(struct parser_imported_node*)imported;
	for (int i = 0; i < mod->name.len; i++) {
		if (cur->nodes[(int)mod->name.s[i]] != NULL) {
			cur = cur->nodes[(int)mod->name.s[i]];
			continue;
		}
		cur->nodes[(int)mod->name.s[i]] = calloc(1, sizeof(*cur));
		cur = cur->nodes[(int)mod->name.s[i]];
	}
	imported->count += 1;
	imported->mods = realloc(imported->mods, sizeof(*imported->mods)
			* imported->count);
	imported->mods[imported->count - 1] = mod;
	cur->mod = mod;
	return 0;
}

yz_module *parser_imported_find(struct parser_imported *imported, str *name)
{
	struct parser_imported_node *cur = (struct parser_imported_node*)imported;
	for (int i = 0; i < name->len; i++) {
		if (cur->nodes[(int)name->s[i]] == NULL)
			return NULL;
		cur = cur->nodes[(int)name->s[i]];
	}
	return cur ? cur->mod : NULL;
}

struct scope *parser_parsed_file_find(str *path)
{
	struct parsed_node *cur = (struct parsed_node*)&global_parser.parsed;
	for (int i = 0; i < path->len; i++) {
		if (cur->nodes[(int)path->s[i]] == NULL)
			return NULL;
		cur = cur->nodes[(int)path->s[i]];
	}
	return cur ? cur->scope : NULL;
}

void free_parser(struct parser *parser)
{
	free_hooks(parser->hooks);
	free_parser_imported(&parser->imported);
	free(parser->path.s);
	free_scope(parser->scope);
	free(parser->target.s);
}

void free_parser_imported(struct parser_imported *imported)
{
	if (imported == NULL)
		return;
	free_parser_imported_mods(imported->count, imported->mods);
	for (int i = 0; i < UCHAR_MAX; i++) {
		if (imported->nodes[i] == NULL)
			continue;
		free_parser_imported_nodes(imported->nodes[i], 0);
	}
}

void free_parser_imported_mods(int count, yz_module **mods)
{
	if (mods == NULL)
		return;
	for (int i = 0; i < count; i++)
		free_yz_module(mods[i]);
	free(mods);
}

void free_parser_imported_nodes(struct parser_imported_node *root,
		int free_mod)
{
	if (root == NULL)
		return;
	for (int i = 0; i < UCHAR_MAX; i++) {
		if (root->nodes[i] == NULL)
			continue;
		free_parser_imported_nodes(root->nodes[i], free_mod);
	}
	if (free_mod)
		free_yz_module(root->mod);
	free(root);
}
