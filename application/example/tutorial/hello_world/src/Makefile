
LIBPATH = ../lib
INCPATH = $(CURDIR)/../include

OBJ_HELLO = $(LIBPATH)/hello_world.o
OBJ_HELLO2 = $(LIBPATH)/hello2.o
OBJ_LIBHELLO = $(LIBPATH)/libhello_util.so

OBJ += $(OBJ_HELLO)
OBJ += $(OBJ_HELLO2)
OBJ += $(OBJ_LIBHELLO)

CXXFLAGS += -I$(INCPATH)
CXXFLAGS += -fPIC

CFLAGS += -I$(INCPATH)
CFLAGS += -fPIC

###############################################################################

.PHONY: default all

default: all
all: $(OBJ)

###############################################################################

$(OBJ_HELLO): hello_world.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_HELLO2): hello2.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_LIBHELLO): hello_util.c
	$(CC) $(CFLAGS) -shared $< -o $@

###############################################################################

.PHONY: bakclean objclean clean

RM = $(shell which rm) -f

clean: bakclean objclean
bakclean:
	-$(RM) *~
objclean:
	-$(RM) $(OBJ)
