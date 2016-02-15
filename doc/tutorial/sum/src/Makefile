
ifndef DIR_BUILD
  $(error DIR_BUILD not set)
endif

LIB_PATH = $(DIR_BUILD)
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
