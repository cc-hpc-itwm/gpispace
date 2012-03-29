default: all

all: gpi-test

CC = g++

CXXFLAGS += -I/p/herc/itwm/hpc/ap/git/sdpa/gpi-space/ext/include
LIBS += /p/herc/itwm/hpc/ap/git/sdpa/gpi-space/ext/lib64/libGPI.a
LIBS += /p/herc/itwm/hpc/ap/git/sdpa/gpi-space/ext/lib64/libibverbs15.so

gpi-test: gpi-test.o $(LIBS)

clean:
	@rm -f gpi-test
	@rm -f *.o

run: gpi-test
	cp gpi-test /opt/cluster/GPI/bin/petry/gpi-test && /opt/cluster/GPI/bin/petry/gpi-test $(ARGS)

.PHONY: clean default all
