include ../config.mk
include config.mk

DEPS = $(CORE_OBJ:.o=.d)

.PHONY: all clean
all: $(CORE_BUILD) $(CORE_OBJ) $(DEPS)

$(CORE_BUILD):
	mkdir -p $@

$(CORE_BUILD)/%.d: %.c | $(CORE_BUILD)
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,$(CORE_BUILD)/\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$ %.c

$(CORE_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CORE_OBJ) $(DEPS)

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(DEPS)
endif
