
MAIN = capacity

PATH_MAIN = $(CURDIR)

###############################################################################

SDPA_HOME = /var/tmp/mr/inst

SDPA_INCLUDE = $(SDPA_HOME)/include
SDPA_BIN     = $(SDPA_HOME)/bin
SDPA_XML_LIB = $(SDPA_HOME)/share/sdpa/xml/lib

TEE   = $(shell which tee)
DOT   = $(shell which dot) -Tps
RM    = $(shell which rm) -rf
TOUCH = $(shell which touch)
CAT   = $(shell which cat)
CP    = $(shell which cp)
MKDIR = $(shell which mkdir)

###############################################################################

XML_MAIN = $(PATH_MAIN)/capacity.xpnet

###############################################################################

DEP += Makefile

NET = $(MAIN).pnet
PUT = $(NET).put
GEN = gen
OUT = $(MAIN).out
PS  = $(MAIN).ps

PATH_LIB = $(PATH_MAIN)/$(GEN)/pnetc/op

LIB = $(PATH_LIB)/lib$(MAIN).so

###############################################################################

NOINLINE_NOT_ENDS_WITH += generate

NOINLINE_NOT_STARTS_WITH += dup
NOINLINE_NOT_STARTS_WITH += scatter
NOINLINE_NOT_STARTS_WITH += eat
NOINLINE_NOT_STARTS_WITH += wait

NET_NOINLINE = $(MAIN).noinline.pnet
PS_NOINLINE = $(MAIN).noinline.ps

###############################################################################

WE_EXEC_LIBPATHS += $(PATH_LIB)

###############################################################################

CXXINCLUDEPATHS += $(SDPA_INCLUDE)

###############################################################################

PNETC    = $(SDPA_BIN)/pnetc
PNET2DOT = $(SDPA_BIN)/pnet2dot
PNETPUT  = $(SDPA_BIN)/pnetput
WE_EXEC  = $(SDPA_BIN)/we-exec

###############################################################################

PNETC += $(addprefix -I,$(SDPA_XML_LIB))
PNETC += --gen-cxxflags=-O3
PNETC += --gen-cxxflags=-I$(SDPA_INCLUDE)
PNETC += --force-overwrite-file=true
PNETC += --Woverwrite-file=false
PNETC += --Wbackup-file=false

WE_EXEC += --w 1
WE_EXEC += $(addprefix --load,$(WE_EXEC_LOAD))
WE_EXEC += $(addprefix -L,$(WE_EXEC_LIBPATHS))

###############################################################################

PNET2DOT_NOINLINE += $(PNET2DOT)
PNET2DOT_NOINLINE += $(addprefix --not-starts-with ,$(NOINLINE_NOT_STARTS_WITH))
PNET2DOT_NOINLINE += $(addprefix --not-ends-with ,$(NOINLINE_NOT_ENDS_WITH))

PNETC_NOINLINE += $(PNETC)
PNETC_NOINLINE += --no-inline true
PNETC_NOINLINE += --synthesize-virtual-places true

###############################################################################

.PHONY: default ps all out net put

default: all

ps: $(PS) $(PS_NOINLINE)

net: $(NET)
put: $(PUT)

all: net $(GEN)

###############################################################################

$(NET): $(XML_MAIN) $(DEP)
	$(PNETC) $(XML_MAIN) -o $@

$(NET_NOINLINE): $(XML_MAIN) $(DEP)
	$(PNETC_NOINLINE) $(XML_MAIN) -o $@

###############################################################################

ifeq ($(N),)
N = 4
endif

ifeq ($(SA),)
SA = 1
endif

ifeq ($(SB),)
SB = 2
endif

PUTTED = $(PUT).$(N).$(SA).$(SB)

$(PUTTED): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUTTED) -p n=$(N) -p sa=$(SA) -p sb=$(SB)

###############################################################################

$(GEN): $(XML_MAIN) $(DEP)
	$(PNETC) $(XML_MAIN) -o /dev/null -g $@
	CXXINCLUDEPATHS="$(CXXINCLUDEPATHS)" $(MAKE) -j -C $(GEN)
	$(TOUCH) $@

###############################################################################

$(PS): $(NET)
	$(PNET2DOT) --input $^ | $(DOT) -o $@

$(PS_NOINLINE): $(NET_NOINLINE)
	$(PNET2DOT_NOINLINE) --input $^ | $(DOT) -o $@

###############################################################################

out: $(GEN) $(PUTTED)
	$(WE_EXEC) --net $(PUTTED) 2>&1 | $(TEE) $(OUT)

###############################################################################

.PHONY: clean

clean:
	$(RM) $(GEN)
	$(RM) $(NET)
	$(RM) $(PUTTED)
	$(RM) $(NET_NOINLINE)
	$(RM) $(PS)
	$(RM) $(PS_NOINLINE)
	$(RM) $(OUT)
