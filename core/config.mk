CORE_SRC =\
	file.c\
	ptr.c\
	scope.c\
	symbol.c\
	token.c\
	type.c
CORE_OBJ = $(CORE_SRC:.c=.o)
CORE_DEBUG_OBJ = $(CORE_SRC:.c=.debug.o)
