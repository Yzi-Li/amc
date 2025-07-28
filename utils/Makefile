include ../config.mk
include config.mk

.PHONY: all clean debug
all: $(UTILS_BUILD) $(UTILS_OBJ)
debug: $(UTILS_BUILD) $(UTILS_DEBUG_OBJ)

$(UTILS_BUILD):
	mkdir -p $@

$(UTILS_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(UTILS_BUILD)/%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

clean:
	rm -f $(UTILS_OBJ) $(UTILS_DEBUG_OBJ)
