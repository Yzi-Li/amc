COMPTIME_SRC =\
	inspector/ptr.c

COMPTIME_OBJ = $(COMPTIME_SRC:.c=.o)
COMPTIME_DEBUG_OBJ = $(COMPTIME_SRC:.c=.debug.o)
