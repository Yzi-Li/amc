/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
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
#include <sctrie.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__)
#include <libgen.h>
#include <unistd.h>
#if defined(__linux__)
#include <linux/limits.h>
#else
#define PATH_MAX 256
#endif
#elif defined(__WIN32)
#error Unsupport platform!
#endif

struct global_parser global_parser = {
	.has_err = 0,
	.has_main = 0,
	.parsed = {},
	.output = {},
	.root_dir = {},
	.root_mod = {},
	.target_path = {}
};

static int parse_line(struct parser *parser);
static int parser_create_get_path(str *result, str *path);

int parse_line(struct parser *parser)
{
	char *err_msg;
	int ret = 0;
	struct symbol *sym = NULL;
	str token = TOKEN_NEW;
	file_skip_space(parser->f);
	if (parse_comment(parser->f))
		return -1;
	if (parser->f->src[parser->f->pos] == '\n')
		return 0;
	if (parser->f->src[parser->f->pos] == '@') {
		parser->stat.decorators.has = 1;
		return parse_decorator(&parser->stat.decorators, parser->f);
	}
	if (token_next(&token, parser->f))
		return 1;
	if (!keyword_find(&token, &sym))
		goto err_sym_not_found;
	if (!sym->flags.toplevel)
		goto err_not_toplevel;
	parser->sym = sym;
	if ((ret = sym->parse_function(parser)) > 0)
		return 1;
	if (parser_stat_restore(&parser->stat))
		return 1;
	return ret;
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
	printf("==> \x1b[32mCompiling\x1b[0m: %s\n", real_path);
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
	result->scope = calloc(1, sizeof(*result->scope));
	result->scope->fn = NULL;
	result->scope->indent = 0;
	result->scope->parent = NULL;
	result->scope->status = NULL;
	result->scope->status_type = SCOPE_TOP;
	result->scope_pub = calloc(1, sizeof(*result->scope_pub));
	result->scope_pub->fn = NULL;
	result->scope_pub->indent = 0;
	result->scope_pub->parent = NULL;
	result->scope_pub->status = NULL;
	result->scope_pub->status_type = SCOPE_TOP;
	result->scope->parent = result->scope_pub;
	result->stat.decorators.hooks
		= calloc(1, sizeof(*result->stat.decorators.hooks));
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
	str pwd = {
		.len = PATH_MAX,
		.s = calloc(PATH_MAX, sizeof(char))
	};
	char *root_dir_cpy;
	if (global_parser.output.s == NULL) {
		global_parser.output.s = "a.out";
		global_parser.output.len = strlen(global_parser.output.s);
	}
	if (global_parser.root_dir.s == NULL) {
		global_parser.root_dir.s = getcwd(pwd.s, pwd.len);
		global_parser.root_dir.len = strlen(global_parser.root_dir.s);
	}
	if (global_parser.root_mod.s == NULL) {
		root_dir_cpy = calloc(global_parser.root_dir.len + 1,
				sizeof(char));
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
	return backend_end(&global_parser.output);
}

int parser_imported_append(struct parser_imported *imported, yz_module *mod)
{
	struct parser_imported_node *cur =
		sctrie_append_elem(imported, sizeof(*cur),
				mod->name.s, mod->name.len);
	imported->count += 1;
	imported->mods = realloc(imported->mods, sizeof(*imported->mods)
			* imported->count);
	imported->mods[imported->count - 1] = mod;
	cur->mod = mod;
	return 0;
}

yz_module *parser_imported_find(struct parser_imported *imported, str *name)
{
	struct parser_imported_node *result =
		sctrie_find_elem(imported, name->s, name->len);
	return result ? result->mod : NULL;
}

struct scope *parser_parsed_file_find(str *path)
{
	struct parsed_node *result =
		sctrie_find_elem(&global_parser.parsed, path->s, path->len);
	return result ? result->scope : NULL;
}

int parser_stat_restore(struct parser_stat *self)
{
	self->has_pub = 0;
	if (self->decorators.has) {
		if (!self->decorators.used)
			goto err_hook_not_used;
		free_decorators_noself(&self->decorators);
	}
	return 0;
err_hook_not_used:
	printf("amc: parser_stat_restore: Decorator defined but not used!\n");
	return 1;
}

void free_parser(struct parser *self)
{
	if (self == NULL)
		return;
	free_parser_stat_noself(&self->stat);
	free_parser_imported(&self->imported);
	free(self->path.s);
	free_scope(self->scope);
	free(self->target.s);
}

void free_parser_imported(struct parser_imported *self)
{
	if (self == NULL)
		return;
	free_parser_imported_mods(self->count, self->mods);
	for (int i = 0; i < UCHAR_MAX; i++) {
		if (self->nodes[i] == NULL)
			continue;
		free_parser_imported_nodes(self->nodes[i], 0);
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

void free_parser_stat_noself(struct parser_stat *self)
{
	if (self == NULL)
		return;
	free_decorators_noself(&self->decorators);
}
