CHECKER_SRC =\
	ptr.c\
	struct.c\
	symbol.c\
	type.c

CHECKER_BUILD = ../build/checker
CHECKER_OBJ = $(addprefix $(CHECKER_BUILD)/, $(CHECKER_SRC:.c=.o))
