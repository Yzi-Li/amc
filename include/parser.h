#ifndef AMC_PARSER_H
#define AMC_PARSER_H
#include "../include/file.h"
#include "../include/module.h"
#include "../include/scope.h"

struct parser {
	struct file *f;
	struct symbol *sym;
	struct hooks *hooks;
	struct scope *scope;
};

struct global_parser {
	unsigned int has_err:1, has_main:1;
	int parsed_file_count;
	const char *target_path;
};

extern struct global_parser global_parser;

int parse_file(const char *path, struct file *f);
int parser_init(const char *path, struct file *f);

#endif
