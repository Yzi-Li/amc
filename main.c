#include "utils/die.h"

#include "include/backend.h"
#include "include/file.h"
#include "include/parser.h"
#include <string.h>

/**
 * Please install 'libgetarg'(https://github.com/at2er/libgetarg)
 */
#include <getarg.h>

#define AMC_VERSION "0.0.0"

static const char *src = NULL;

static int err_no_input();
static int opt_output(int argc, char *argv[], struct option *opt);
static int opt_read_src(int argc, char *argv[], struct option *opt);
static int opt_root_mod(int argc, char *argv[], struct option *opt);
static int print_version();

int err_no_input()
{
	die("amc: \x1b[31merror\x1b[0m: no input file!\n");
	return 1;
}

int opt_output(int argc, char *argv[], struct option *opt)
{
	if (argc > 1 || argv == NULL)
		goto err_too_many_files;
	global_parser.output.s = argv[0];
	global_parser.output.len = strlen(argv[0]);
	return 0;
err_too_many_files:
	printf("amc: opt_read_src: Unsupport multiple entry files!\n");
	return 1;
}

int opt_read_src(int argc, char *argv[], struct option *opt)
{
	if (argc > 1 || argv == NULL)
		goto err_too_many_files;
	src = argv[0];
	return 0;
err_too_many_files:
	printf("amc: opt_read_src: Unsupport multiple entry files!\n");
	return 1;
}

int opt_root_mod(int argc, char *argv[], struct option *opt)
{
	if (argc != 1 || argv == NULL)
		return 1;
	global_parser.root_mod.s = argv[0];
	global_parser.root_mod.len = strlen(global_parser.root_mod.s);
	return 0;
}

int print_version()
{
	die("Atom compiler(Yuan Zi Compiler) v%s\n", AMC_VERSION);
	return 0;
}

int main(int argc, char *argv[])
{
	struct file f = {};
	struct option options[] = {
		{
			"help", 'h',
			GETARG_HELP_OPT, 0,
			NULL,
			"show help documents",
			NULL
		},
		{
			"output", 'o',
			GETARG_LIST_ARG, 0,
			opt_output,
			"output file name",
			NULL
		},
		{
			"root-mod", '\0',
			GETARG_LIST_ARG, 0,
			opt_root_mod,
			"set root module name",
			NULL
		},
		{
			NULL, '\0',
			GETARG_LIST_ARG, 0,
			opt_read_src,
			"source file",
			NULL
		}
	};

	if (getarg(argc, argv, options))
		return 1;

	if (backend_init(argc, argv))
		die("amc: backend_init: cannot init backend.");
	if (argc < 2)
		return print_version();
	if (src == NULL)
		return err_no_input();
	return parser_init(src, &f);
}
