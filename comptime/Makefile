include ../config.mk
include config.mk

.PHONY: all clean debug
all: $(COMPTIME_OBJ)
debug: $(COMPTIME_DEBUG_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

clean:
	rm -f $(COMPTIME_OBJ) $(COMPTIME_DEBUG_OBJ)
