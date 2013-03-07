
ifndef DIR_BUILD
  DIR_BUILD=..
endif

LIB_PATH = $(DIR_BUILD)/lib
INC_PATH = $(CURDIR)/../include

OBJ = $(LIB_PATH)/sum.o

CFLAGS += -I$(INC_PATH)
CFLAGS += -fPIC

###############################################################################

.PHONY: default all

default: all
all: $(OBJ)

###############################################################################

$(LIB_PATH):
	@mkdir -p "$@"

$(OBJ): sum.c $(LIB_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

###############################################################################

.PHONY: bakclean objclean clean

ifndef RM
  RM = $(shell which rm) -f
endif

clean: bakclean objclean
bakclean:
	-$(RM) *~
objclean:
	-$(RM) $(OBJ)
	-$(RM) -r $(LIB_PATH)
