include config.mk
include utils/config.mk

include core/config.mk

# backend
include backend/config.mk

include comptime/config.mk
include parser/config.mk

UTILSDIR  = utils
COREDIR   = core
COMPTIME  = comptime
PARSER    = parser
BACKEND   = backend

BUILD          = build

UTILS_TARGET          = $(UTILS_OBJ:../%=%)
UTILS_DEBUG_TARGET    = $(UTILS_DEBUG_OBJ:../%=%)
CORE_TARGET           = $(CORE_OBJ:../%=%)
CORE_DEBUG_TARGET     = $(CORE_DEBUG_OBJ:../%=%)
COMPTIME_TARGET       = $(COMPTIME_OBJ:../%=%)
COMPTIME_DEBUG_TARGET = $(COMPTIME_DEBUG_OBJ:../%=%)
PARSER_TARGET         = $(PARSER_OBJ:../%=%)
PARSER_DEBUG_TARGET   = $(PARSER_DEBUG_OBJ:../%=%)
BACKEND_TARGET        = $(BACKEND_OBJ:../%=%)
BACKEND_DEBUG_TARGET  = $(BACKEND_DEBUG_OBJ:../%=%)

SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = amc
DEBUG_OBJ = $(SRC:.c=.debug.o)
DEBUG_TARGET = $(TARGET).debug

# libs
include lib/libgetarg.mk
include lib/libsctrie.mk

STRDIR = $(UTILSDIR)/str
STRLIB = $(STRDIR)/libstr.a
CLIBS  = -L$(STRDIR) -lstr \
	$(LIBGETARG) $(LIBSCTRIE)

.PHONY: all clean debug debug_target
.PHONY: $(BUILD) $(STRLIB)
.PHONY: $(COREDIR) $(COMPTIME) $(PARSER) $(BACKEND) $(UTILSDIR)
all: $(TARGET)
debug: $(DEBUG_TARGET)
debug_target:
	@$(MAKE) -C $(COREDIR) debug
	@$(MAKE) -C $(COMPTIME) debug
	@$(MAKE) -C $(PARSER) debug
	@$(MAKE) -C $(BACKEND) debug
	@$(MAKE) -C $(UTILSDIR) debug

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

$(STRLIB):
	@$(MAKE) -C $(STRDIR)

$(COREDIR) $(COMPTIME) $(PARSER) $(BACKEND) $(UTILSDIR):
	@$(MAKE) -C $@

$(TARGET): $(BUILD) $(OBJ) $(COREDIR) $(COMPTIME) $(PARSER) $(BACKEND) \
	$(UTILSDIR) $(STRLIB)
	$(CC) $(CFLAGS) -o $@ \
		$(OBJ) \
		$(CORE_TARGET) \
		$(COMPTIME_TARGET) \
		$(PARSER_TARGET) \
		$(UTILS_TARGET) \
		$(BACKEND_TARGET) \
		$(CLIBS)

$(DEBUG_TARGET): $(BUILD) $(DEBUG_OBJ) debug_target $(STRLIB)
	$(CC) $(CFLAGS) $(CDEBUG) -o $@\
		$(DEBUG_OBJ)\
		$(CORE_DEBUG_TARGET)\
		$(COMPTIME_DEBUG_TARGET)\
		$(PARSER_DEBUG_TARGET)\
		$(UTILS_DEBUG_TARGET)\
		$(BACKEND_DEBUG_TARGET)\
		$(CLIBS)

clean:
	rm -f $(TARGET) $(OBJ) $(DEBUG_TARGET) $(DEBUG_OBJ)
	@$(MAKE) -C $(COREDIR) clean
	@$(MAKE) -C $(COMPTIME) clean
	@$(MAKE) -C $(STRDIR) clean
	@$(MAKE) -C $(PARSER) clean
	@$(MAKE) -C $(BACKEND) clean
	@$(MAKE) -C $(UTILSDIR) clean
