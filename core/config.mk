CORE_SRC =\
	array.c\
	expr.c\
	file.c\
	module.c\
	ptr.c\
	scope.c\
	struct.c\
	symbol.c\
	token.c\
	type.c\
	val.c
CORE_OBJ = $(CORE_SRC:.c=.o)
CORE_DEBUG_OBJ = $(CORE_SRC:.c=.debug.o)
