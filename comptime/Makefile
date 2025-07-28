include ../config.mk
include config.mk

.PHONY: all clean debug
all: $(COMPTIME_BUILD) $(COMPTIME_OBJ)
debug: $(COMPTIME_BUILD) $(COMPTIME_DEBUG_OBJ)

$(COMPTIME_BUILD):
	mkdir -p $@
	mkdir -p $@/inspector

$(COMPTIME_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(COMPTIME_BUILD)/%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

clean:
	rm -f $(COMPTIME_OBJ) $(COMPTIME_DEBUG_OBJ)
