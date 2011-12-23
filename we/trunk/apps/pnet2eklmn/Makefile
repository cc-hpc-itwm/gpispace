EXAMPLE		= kdm
EXAMPLE_PNET	= $(EXAMPLE).pnet
EXAMPLE_PS	= $(EXAMPLE).ps
EXAMPLE_EKLMN	= $(EXAMPLE).tcl
EXAMPLE_XML	= ~/p/sdpa/git/trunk/application/kdm/pnet/full/kdm.xml
#EXAMPLE_XML	= ~/p/sdpa/git/trunk/xml/example/use_sequence.xml
PNETC_FLAGS	= -I ~/p/sdpa/git/trunk/application/lib --no-inline=0

.PHONY: all
all: $(EXAMPLE_PS) $(EXAMPLE_EKLMN)

.PHONY: pnet2eklmn
pnet2eklmn:
	make -C ~/p/sdpa/build-lts-release pnet2eklmn

$(EXAMPLE_PNET):
	pnetc $(PNETC_FLAGS) $(EXAMPLE_XML) > $(EXAMPLE_PNET)

$(EXAMPLE_PS): $(EXAMPLE_PNET)
	pnet2dot $(EXAMPLE_PNET) | dot -Tps -Grankdir=LR > $(EXAMPLE_PS)

$(EXAMPLE_EKLMN): $(EXAMPLE_PNET) pnet2eklmn
	~/bin/pnet2eklmn $(EXAMPLE_PNET) -o xxx

.PHONY: clean
clean:
	-rm $(EXAMPLE_PNET) $(EXAMPLE_PS) $(EXAMPLE_EKLMN) core
