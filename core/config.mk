CORE_SRC =\
	file.c\
	token.c\
	symbol.c\
	type.c
CORE_OBJ = $(CORE_SRC:.c=.o)
CORE_DEBUG_OBJ = $(CORE_SRC:.c=.debug.o)
