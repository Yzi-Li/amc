#ifndef AMC_PARSER_H
#define AMC_PARSER_H
#include "file.h"
#include "module.h"
#include "scope.h"
#include <limits.h>

struct parsed_node {
	struct parsed_node *nodes[UCHAR_MAX];
	struct scope *scope;
};

struct parsed_list {
	struct parsed_node *nodes[UCHAR_MAX];
};

struct parser_imported_node {
	struct parser_imported_node *nodes[UCHAR_MAX];
	yz_module *mod;
};

struct parser_imported {
	struct parser_imported_node *nodes[UCHAR_MAX];
	int count;
	yz_module **mods;
};

struct parser {
	struct file *f;
	struct hooks *hooks;
	struct parser_imported imported;
	str path;
	struct scope *scope;
	struct symbol *sym;
	str target;
};

struct global_parser {
	unsigned int has_err:1, has_main:1;
	struct parsed_list parsed;
	str root_dir, root_mod, target_path;
};

extern struct global_parser global_parser;

struct parser *parse_file(str *path, const char *real_path, struct file *f);
struct parser *parser_create(str *path, const char *real_path, struct file *f);
int parser_get_target_from_mod_path(str *result, str *path);
int parser_imported_append(struct parser_imported *imported, yz_module *mod);
yz_module *parser_imported_find(struct parser_imported *imported, str *name);
int parser_init(const char *path, struct file *f);
struct scope *parser_parsed_file_find(str *path);

void free_parser(struct parser *parser);

#endif
