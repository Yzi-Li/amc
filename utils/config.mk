UTILS_SRC =\
	converter.c\
	die.c\
	utils.c

UTILS_BUILD = ../build/utils
UTILS_OBJ = $(addprefix $(UTILS_BUILD)/, $(UTILS_SRC:.c=.o))
UTILS_DEBUG_OBJ = $(addprefix $(UTILS_BUILD)/, $(UTILS_SRC:.c=.debug.o))
