PARSER_SRC =\
	array.c\
	block.c\
	comment.c\
	constructor.c\
	decorator.c\
	func.c\
	expr.c\
	identifier.c\
	if.c\
	indent.c\
	keywords.c\
	loop.c\
	match.c\
	module.c\
	op.c\
	parser.c\
	ptr.c\
	struct.c\
	type.c\
	utils.c
PARSER_OBJ = $(PARSER_SRC:.c=.o)
PARSER_DEBUG_OBJ = $(PARSER_SRC:.c=.debug.o)
