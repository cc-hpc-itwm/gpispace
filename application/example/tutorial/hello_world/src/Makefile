
OBJ += ../lib/hello_world.o
OBJ += ../lib/hello2.o
OBJ += ../lib/libhello_util.so

CXXFLAGS += -I$(CURDIR)/../include
CXXFLAGS += -fPIC

CFLAGS += -I$(CURDIR)/../include
CFLAGS += -fPIC

../lib/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

../lib/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

../lib/libhello_util.so: hello_util.c
	$(CC) $(CFLAGS) -shared $< -o $@

###############################################################################

.PHONY: default all

default: all
all: $(OBJ)

###############################################################################

.PHONY: bakclean objclean clean

clean: bakclean objclean
bakclean:
	-rm -f *~
objclean:
	-rm -f $(OBJ)
