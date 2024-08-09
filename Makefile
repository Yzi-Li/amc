include config.mk
include utils/config.mk

include core/config.mk

# backend
include backend/config.mk

include parser/config.mk

UTILSDIR = utils
STRDIR = $(UTILSDIR)/str
STRLIB = $(STRDIR)/libstr.a
PARSERDIR = parser
COREDIR = core
BACKEND = backend
UTILS_TARGET = $(addprefix $(UTILSDIR)/, $(UTILS_OBJ))
UTILS_DEBUG_TARGET = $(addprefix $(UTILSDIR)/, $(UTILS_DEBUG_OBJ))
CORE_TARGET = $(addprefix $(COREDIR)/, $(CORE_OBJ))
CORE_DEBUG_TARGET = $(addprefix $(COREDIR)/, $(CORE_DEBUG_OBJ))
PARSER_TARGET = $(addprefix $(PARSERDIR)/, $(PARSER_OBJ))
PARSER_DEBUG_TARGET = $(addprefix $(PARSERDIR)/, $(PARSER_DEBUG_OBJ))
BACKEND_TARGET = $(addprefix backend/, $(BACKEND_OBJ))
BACKEND_DEBUG_TARGET = $(addprefix backend/, $(BACKEND_DEBUG_OBJ))

SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = amc
DEBUG_OBJ = $(SRC:.c=.debug.o)
DEBUG_TARGET = $(TARGET).debug

CLIBS = -L$(STRDIR) -lstr

.PHONY: all clean debug debug_target
.PHONY: $(COREDIR) $(PARSERDIR) $(BACKEND) $(STRLIB) $(UTILSDIR)
all: $(TARGET)
debug: $(DEBUG_TARGET)
debug_target:
	@$(MAKE) -C $(COREDIR) debug
	@$(MAKE) -C $(PARSERDIR) debug
	@$(MAKE) -C $(BACKEND) debug
	@$(MAKE) -C $(UTILSDIR) debug

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.debug.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) -c $< -o $@

$(STRLIB):
	@$(MAKE) -C $(STRDIR)

$(COREDIR) $(PARSERDIR) $(BACKEND) $(UTILSDIR):
	@$(MAKE) -C $@

$(TARGET): $(OBJ) $(COREDIR) $(PARSERDIR) $(BACKEND) $(UTILSDIR) $(STRLIB)
	$(CC) $(CFLAGS) -o $@\
		$(OBJ)\
		$(CORE_TARGET)\
		$(PARSER_TARGET)\
		$(UTILS_TARGET)\
		$(BACKEND_TARGET)\
		$(CLIBS)

$(DEBUG_TARGET): $(DEBUG_OBJ) debug_target $(STRLIB)
	$(CC) $(CFLAGS) $(CDEBUG) -o $@\
		$(DEBUG_OBJ)\
		$(CORE_DEBUG_TARGET)\
		$(PARSER_DEBUG_TARGET)\
		$(UTILS_DEBUG_TARGET)\
		$(BACKEND_DEBUG_TARGET)\
		$(CLIBS)

clean:
	rm -f $(TARGET) $(OBJ) $(DEBUG_TARGET) $(DEBUG_OBJ)
	@$(MAKE) -C $(COREDIR) clean
	@$(MAKE) -C $(STRDIR) clean
	@$(MAKE) -C $(PARSERDIR) clean
	@$(MAKE) -C $(BACKEND) clean
	@$(MAKE) -C $(UTILSDIR) clean
