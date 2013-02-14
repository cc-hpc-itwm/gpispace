ifndef XSLT
	XSLT = $(shell which xsltproc 2>/dev/null)
endif

ifndef TRANG
	TRANG = $(shell which trang 2>/dev/null)
endif

XSD := pnet.xsd

RNC = $(XSD:.xsd=.rnc)

default: pnet.xsd patch
all: $(RNC)

help:
	@echo "Convert XSD schema files to RNC"
	@echo ""
	@echo "   available targets:"
	@echo ""
	@echo "     default: all"
	@echo "         all: convert all XSD to RNC"
	@echo "       clean: remove generated files"
	@echo ""
	@echo ""
	@echo "   variables to set:"
	@echo ""
	@echo "       XSD: the schema file to convert"
	@echo "      XSLT: path to the 'xsltproc' command"
	@echo "     TRANG: path to the 'trang' command"

ifeq "$(TRANG)" ""
	$(error 'trang' command line tool is required)
endif

ifeq "$(XSLT)" ""
	$(error 'xsltproc' command line tool is required)
endif

%.rnc : %.rng ; $(TRANG) -I rng -O rnc $^ $@
%.rng : %.xsd ; $(XSLT) -o $@ XSDtoRNG.xsl $^

patch: pnet.rnc pnet.rnc.fix.patch
	patch -p1 < pnet.rnc.fix.patch

.PHONY: default all clean help patch

clean:
	rm -f $(RNC)
