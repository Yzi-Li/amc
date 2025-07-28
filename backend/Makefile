include ../config.mk
include config.mk

.PHONY: all clean debug
all: $(BACKEND_BUILD) $(BACKEND_OBJ)
debug: $(BACKEND_BUILD) $(BACKEND_DEBUG_OBJ)

$(BACKEND_BUILD):
	mkdir -p $@
	mkdir -p $@/asf

$(BACKEND_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BACKEND_BUILD)/%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

clean:
	rm -f $(BACKEND_OBJ) $(BACKEND_DEBUG_OBJ)
