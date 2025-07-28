include ../config.mk
include config.mk

.PHONY: all clean debug
all: $(CORE_BUILD) $(CORE_OBJ)
debug: $(CORE_BUILD) $(CORE_DEBUG_OBJ)

$(CORE_BUILD):
	mkdir -p $@

$(CORE_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(CORE_BUILD)/%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

clean:
	rm -f $(CORE_OBJ) $(CORE_DEBUG_OBJ)
