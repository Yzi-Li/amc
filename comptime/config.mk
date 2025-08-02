COMPTIME_SRC =\
	inspector/ptr.c\
	inspector/struct.c\
	inspector/symbol.c\
	inspector/type.c\
	hook.c

COMPTIME_BUILD = ../build/comptime
COMPTIME_OBJ = $(addprefix $(COMPTIME_BUILD)/, $(COMPTIME_SRC:.c=.o))
