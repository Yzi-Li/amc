#include "utils/args.h"
#include "utils/die.h"
#include <stdio.h>

#define ATOM_VERSION "0.0.0"

static int print_version(str *arg, int index, int argc, char *argv[]);
static int source_comp(str *arg, int index, int argc, char *argv[]);

#include "include/backend.h"
#include "include/file.h"
#include "include/parser.h"

#include "parser/keywords.h"

int print_version(str *arg, int index, int argc, char *argv[])
{
	die("Yuan Zi Compiler v%s\n", ATOM_VERSION);
	return 0;
}

int source_comp(str *arg, int index, int argc, char *argv[])
{
	struct file f;
	str_append(arg, 1, "\0");
	parser_init(arg->s, &f);
	return 0;
}

int main(int argc, char *argv[])
{
	struct file f = {};
	struct option options[] = {
		{"v",  print_version, 0, 0, 0},
		{"s",  source_comp,   1, 1, 0},
		//{NULL, source_comp,   0, 0, 0},
		{NULL, NULL,          0, 0, 0}
	};

	symbols_init();
	keyword_init();
	if (backend_init(argc, argv))
		die("amc: backend_init: cannot init backend.");
	if (argc < 2)
		return print_version(NULL, 0, 0, NULL);
	return parser_init(argv[1], &f);
	//args_apply(options, argc, argv);
}
