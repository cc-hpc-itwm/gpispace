
MAIN = hello_many
PATH_MAIN = $(CURDIR)

CXXINCLUDEPATHS += $(PATH_MAIN)/include

APPLIBRARYPATH = $(PATH_MAIN)/lib

###############################################################################

SDPA_INCLUDE = $(SDPA_HOME)/include
SDPA_BIN = $(SDPA_HOME)/bin
SDPA_XML_LIB = $(SDPA_HOME)/share/sdpa/xml/lib

TEE = $(shell which tee)
DOT = $(shell which dot) -Tps
RM = $(shell which rm) -rf
TOUCH = $(shell which touch)
CAT = $(shell which cat)
CP = $(shell which cp)
MKDIR = $(shell which mkdir)

###############################################################################

XML_MAIN = $(PATH_MAIN)/$(MAIN).xpnet

###############################################################################

DEP += Makefile
DEP += $(PATH_MAIN)/hello_world.xpnet

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
WE_EXEC  = $(SDPA_BIN)/we-eval

###############################################################################

PNETC += $(addprefix -I,$(SDPA_XML_LIB))
PNETC += --gen-cxxflags=-O3
PNETC += $(addprefix --gen-cxxflags=-I,$(CXXINCLUDEPATHS))
PNETC += --gen-ldflags=-L$(APPLIBRARYPATH)
PNETC += --force-overwrite-file=true
PNETC += --Woverwrite-file=false
PNETC += --Wbackup-file=false

WE_EXEC += --w 4
WE_EXEC += $(addprefix --load,$(WE_EXEC_LOAD))
WE_EXEC += $(addprefix -L,$(WE_EXEC_LIBPATHS))
WE_EXEC += -o /dev/null

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

all: net lib

###############################################################################

$(NET): $(XML_MAIN) $(DEP)
	$(PNETC) $(XML_MAIN) -o $@

$(NET_NOINLINE): $(XML_MAIN) $(DEP)
	$(PNETC_NOINLINE) $(XML_MAIN) -o $@

###############################################################################

PUTTED = $(PUT)

$(PUTTED): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUTTED) -p in=[] -p in=[]

###############################################################################

$(GEN): $(XML_MAIN) $(DEP)
	$(PNETC) $(XML_MAIN) -o /dev/null -g $@
	$(TOUCH) $@

.PHONY: lib

lib: $(GEN)
	$(MAKE) -C $(GEN)

###############################################################################

$(PS): $(NET)
	$(PNET2DOT) --input $^ | $(DOT) -o $@

$(PS_NOINLINE): $(NET_NOINLINE)
	$(PNET2DOT_NOINLINE) --input $^ | $(DOT) -o $@

###############################################################################

out: lib $(PUTTED)
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(APPLIBRARYPATH) \
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
