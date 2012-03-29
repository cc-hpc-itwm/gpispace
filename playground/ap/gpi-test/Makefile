default: all

all: gpi-test

CC = g++

CXXFLAGS += -I$(GPI_INCLUDE_DIR)
LIBS += $(GPI_LIBRARY_DIR)/libGPI.a
LIBS += $(GPI_LIBRARY_DIR)/libibverbs15.so

gpi-test: gpi-test.o $(LIBS)

clean:
	@rm -f gpi-test
	@rm -f *.o

run: gpi-test
	cp gpi-test $(DESTDIR)/gpi-test && $(DESTDIR)/gpi-test $(ARGS)

.PHONY: clean default all
