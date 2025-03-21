#include "utils/die.h"

#include "include/backend.h"
#include "include/file.h"
#include "include/parser.h"

#define ATOM_VERSION "0.0.0"

static int print_version();

int print_version()
{
	die("Yuan Zi Compiler v%s\n", ATOM_VERSION);
	return 0;
}

int main(int argc, char *argv[])
{
	struct file f = {};

	if (backend_init(argc, argv))
		die("amc: backend_init: cannot init backend.");
	if (argc < 2)
		return print_version();
	return parser_init(argv[1], &f);
	//args_apply(options, argc, argv);
}
