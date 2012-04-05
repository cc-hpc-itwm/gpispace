default: all

all: gpi-test gpi-exec

gpi-test: gpi-test.cpp -lGPI -libverbs
gpi-exec: gpi-exec.cpp -lGPI -libverbs

clean:
	@rm -f gpi-test
	@rm -f *.o

install: $(DESTDIR)/gpi-test

$(DESTDIR)/gpi-test: gpi-test
	cp gpi-test $(DESTDIR)/gpi-test

run: install
	@$(DESTDIR)/gpi-test $(ARGS)

.PHONY: clean default all run install
