COMPTIME_SRC =\
	inspector/ptr.c\
	inspector/struct.c\
	inspector/symbol.c\
	inspector/type.c\
	hook.c

COMPTIME_OBJ = $(COMPTIME_SRC:.c=.o)
COMPTIME_DEBUG_OBJ = $(COMPTIME_SRC:.c=.debug.o)
