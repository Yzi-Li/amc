include ../config.mk
include config.mk

DEPS = $(CHECKER_OBJ:.o=.d)

.PHONY: all clean
all: $(CHECKER_BUILD) $(CHECKER_OBJ) $(DEPS)

$(CHECKER_BUILD):
	mkdir -p $@

$(CHECKER_BUILD)/%.d: %.c | $(CHECKER_BUILD)
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,$(CHECKER_BUILD)/\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$ %.c

$(CHECKER_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CHECKER_OBJ) $(DEPS)

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(DEPS)
endif
