include ../config.mk
include config.mk

DEPS = $(UTILS_OBJ:.o=.d)

.PHONY: all clean
all: $(UTILS_BUILD) $(UTILS_OBJ) $(DEPS)

$(UTILS_BUILD):
	mkdir -p $@

$(UTILS_BUILD)/%.d: %.c | $(UTILS_BUILD)
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,$(UTILS_BUILD)/\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$ %.c

$(UTILS_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(UTILS_OBJ) $(DEPS)

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(DEPS)
endif
