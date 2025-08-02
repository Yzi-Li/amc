include backend/config.mk
include comptime/config.mk
include config.mk
include core/config.mk
include parser/config.mk
include utils/config.mk

# libs
include lib/libgetarg.mk
include lib/libsctrie.mk
CLIBS  = -L$(STRDIR) -lstr \
	$(LIBGETARG) $(LIBSCTRIE)
STRDIR = $(UTILS)/str
STRLIB = $(STRDIR)/libstr.a

OBJ = $(SRC:.c=.o)
PREFIX = /usr/local
SRC = main.c
TARGET = amc

BUILD       = build
UTILS       = utils
CORE        = core
COMPTIME    = comptime
PARSER      = parser
BACKEND     = backend
MODULES     = $(UTILS) $(CORE) $(COMPTIME) $(PARSER) $(BACKEND)
MODULES_OBJ = $(UTILS_OBJ)\
              $(CORE_OBJ)\
              $(COMPTIME_OBJ)\
              $(PARSER_OBJ)\
              $(BACKEND_OBJ)
OBJ += $(MODULES_OBJ:../%=%)

.PHONY: all clean install
.PHONY: $(STRLIB) $(MODULES)
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

$(STRLIB):
	@$(MAKE) -C $(STRDIR)

$(MODULES): $(BUILD)
	@$(MAKE) -C $@
$(MODULES_OBJ): $(MODULES)

$(TARGET): $(MODULES_OBJ) $(OBJ) $(STRLIB)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(CLIBS)

clean:
	rm -f $(TARGET) $(OBJ)
	@$(MAKE) -C $(CORE) clean
	@$(MAKE) -C $(COMPTIME) clean
	@$(MAKE) -C $(STRDIR) clean
	@$(MAKE) -C $(PARSER) clean
	@$(MAKE) -C $(BACKEND) clean
	@$(MAKE) -C $(UTILS) clean

install: all
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)
