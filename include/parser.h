#ifndef AMC_PARSER_H
#define AMC_PARSER_H
#include "../include/file.h"

struct parser {
	unsigned int has_err:1, has_main:1;
	char *target_name;
};

extern struct parser parser_global_conf;

int parser_init(const char* path, struct file* f);

#endif
