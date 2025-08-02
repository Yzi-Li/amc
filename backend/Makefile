include ../config.mk
include config.mk

DEPS = $(BACKEND_OBJ:.o=.d)

.PHONY: all clean
all: $(BACKEND_BUILD) $(BACKEND_OBJ) $(DEPS)

$(BACKEND_BUILD):
	mkdir -p $@
	mkdir -p $@/asf

$(BACKEND_BUILD)/%.d: %.c | $(BACKEND_BUILD)
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,$(BACKEND_BUILD)/\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$ %.c

$(BACKEND_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BACKEND_OBJ) $(DEPS)

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(DEPS)
endif
