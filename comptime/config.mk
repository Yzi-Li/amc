COMPTIME_SRC =\
	hook.c

COMPTIME_BUILD = ../build/comptime
COMPTIME_OBJ = $(addprefix $(COMPTIME_BUILD)/, $(COMPTIME_SRC:.c=.o))
