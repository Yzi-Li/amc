include ../config.mk
include config.mk

DEPS = $(PARSER_OBJ:.o=.d)

.PHONY: all clean
all: $(PARSER_BUILD) $(PARSER_OBJ) $(DEPS)

$(PARSER_BUILD):
	mkdir -p $@

$(PARSER_BUILD)/%.d: %.c | $(PARSER_BUILD)
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,$(PARSER_BUILD)/\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$ %.c

$(PARSER_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(DEPS) $(PARSER_OBJ)

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(DEPS)
endif
