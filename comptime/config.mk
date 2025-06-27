COMPTIME_SRC =\
	inspector/mut.c\
	inspector/ptr.c\
	inspector/type.c\
	inspector/val.c\
	hook.c

COMPTIME_OBJ = $(COMPTIME_SRC:.c=.o)
COMPTIME_DEBUG_OBJ = $(COMPTIME_SRC:.c=.debug.o)
