UTILS_SRC =\
	converter.c\
	die.c\
	utils.c

UTILS_OBJ = $(UTILS_SRC:.c=.o)
UTILS_DEBUG_OBJ = $(UTILS_SRC:.c=.debug.o)
