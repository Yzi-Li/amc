include ../config.mk
include config.mk

DEPS = $(COMPTIME_OBJ:.o=.d)

.PHONY: all clean
all: $(COMPTIME_BUILD) $(COMPTIME_OBJ) $(DEPS)

$(COMPTIME_BUILD):
	mkdir -p $@

$(COMPTIME_BUILD)/%.d: %.c | $(COMPTIME_BUILD)
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,$(COMPTIME_BUILD)/\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$ %.c

$(COMPTIME_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COMPTIME_OBJ) $(DEPS)

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(DEPS)
endif
