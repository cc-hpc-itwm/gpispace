
ifndef BUILDDIR
  $(error BUILDDIR not set)
endif

LIB_PATH = $(BUILDDIR)
INC_PATH = $(CURDIR)/../include

OBJ_HELLO = $(LIB_PATH)/hello_world.o
OBJ_HELLO2 = $(LIB_PATH)/hello2.o
OBJ_LIBHELLO = $(LIB_PATH)/libhello_util.so

OBJ += $(OBJ_HELLO)
OBJ += $(OBJ_HELLO2)
OBJ += $(OBJ_LIBHELLO)

CXXFLAGS += -I$(INC_PATH)
CXXFLAGS += -fPIC

CFLAGS += -I$(INC_PATH)
CFLAGS += -fPIC

###############################################################################

.PHONY: default all

default: all
all: $(OBJ)

###############################################################################

$(LIB_PATH):
	@mkdir -p "$@"

$(OBJ_HELLO): hello_world.cpp $(LIB_PATH)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_HELLO2): hello2.c $(LIB_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_LIBHELLO): hello_util.c $(LIB_PATH)
	$(CC) $(CFLAGS) -shared $< -o $@
