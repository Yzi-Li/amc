PARSER_SRC =\
	array.c\
	block.c\
	comment.c\
	decorator.c\
	func.c\
	expr.c\
	identifier.c\
	if.c\
	keywords.c\
	loop.c\
	match.c\
	op.c\
	parser.c\
	ptr.c\
	type.c
PARSER_OBJ = $(PARSER_SRC:.c=.o)
PARSER_DEBUG_OBJ = $(PARSER_SRC:.c=.debug.o)
