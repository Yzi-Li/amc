include ../config.mk
include config.mk

.PHONY: all clean debug
all: $(DECORATOR_OBJ)
debug: $(DECORATOR_DEBUG_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

clean:
	rm -f $(DECORATOR_OBJ) $(DECORATOR_DEBUG_OBJ)
