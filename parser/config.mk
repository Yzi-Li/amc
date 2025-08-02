PARSER_SRC =\
	array.c\
	block.c\
	cache.c\
	comment.c\
	constructor.c\
	decorator.c\
	enum.c\
	expr.c\
	func.c\
	identifier.c\
	if.c\
	indent.c\
	keywords.c\
	let.c\
	loop.c\
	match.c\
	module.c\
	op.c\
	parser.c\
	ptr.c\
	struct.c\
	symbol.c\
	type.c\
	utils.c

PARSER_BUILD = ../build/parser
PARSER_OBJ = $(addprefix $(PARSER_BUILD)/, $(PARSER_SRC:.c=.o))
