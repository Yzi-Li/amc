#include "utils/die.h"

#include "include/backend.h"
#include "include/file.h"
#include "include/parser.h"

/**
 * Please install 'libgetarg'(https://github.com/at2er/libgetarg)
 */
#include <getarg.h>

#define AMC_VERSION "0.0.0"

static const char *src = NULL;

static int opt_read_output(int argc, char *argv[], struct option *opt);
static int opt_read_src(int argc, char *argv[], struct option *opt);
static int print_version();

int opt_read_output(int argc, char *argv[], struct option *opt)
{
	if (argc != 1 || argv == NULL)
		return 1;
	parser_global_conf.target_name = argv[0];
	return 0;
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
			opt_read_output,
			"output file",
			NULL
		},
		{
			"src", 's',
			GETARG_LIST_ARG, 0,
			opt_read_src,
			"source file",
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
	return parser_init(src, &f);
}
