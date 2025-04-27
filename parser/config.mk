PARSER_SRC =\
	block.c\
	comment.c\
	func.c\
	expr.c\
	identifier.c\
	if.c\
	keywords.c\
	loop.c\
	match.c\
	parser.c
PARSER_OBJ = $(PARSER_SRC:.c=.o)
PARSER_DEBUG_OBJ = $(PARSER_SRC:.c=.debug.o)
