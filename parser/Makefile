include ../config.mk
include config.mk

.PHONY: all clean debug
all: $(PARSER_BUILD) $(PARSER_OBJ)
debug: $(PARSER_BUILD) $(PARSER_DEBUG_OBJ)

$(PARSER_BUILD):
	mkdir -p $@

$(PARSER_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PARSER_BUILD)/%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

clean:
	rm -f $(PARSER_OBJ) $(PARSER_DEBUG_OBJ)
