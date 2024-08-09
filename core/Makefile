include ../config.mk
include config.mk

.PHONY: all clean debug
all: $(CORE_OBJ)
debug: $(CORE_DEBUG_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

clean:
	rm -f $(CORE_OBJ) $(CORE_DEBUG_OBJ)
