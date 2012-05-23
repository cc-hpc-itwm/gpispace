
ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

###############################################################################

ifndef TEE
  TEE = $(shell which tee)
endif

ifndef DOT
  DOT = $(shell which dot) -Tps
endif

ifndef RM
  RM = $(shell which rm) -f
endif

ifndef TOUCH
  TOUCH = $(shell which touch)
endif

###############################################################################

ifndef SDPA_INCLUDE
  SDPA_INCLUDE = $(SDPA_HOME)/include
endif

ifndef SDPA_BIN
  SDPA_BIN = $(SDPA_HOME)/bin
endif

ifndef SDPA_XML_LIB
  SDPA_XML_LIB = $(SDPA_HOME)/share/sdpa/xml/lib
endif

###############################################################################

ifndef XML
  XML = $(CURDIR)/$(MAIN).xpnet
endif

ifndef NET
  NET = $(MAIN).pnet
endif

ifndef NET_NOINLINE
  NET_NOINLINE = $(MAIN).noinline.pnet
endif

ifndef PUT
  PUT = $(NET).put
endif

ifndef GEN
  GEN = gen
endif

ifndef OUT
  OUT = $(MAIN).out
endif

ifndef PS
  PS = $(MAIN).ps
endif

ifndef PS_NOINLINE
  PS_NOINLINE = $(MAIN).noinline.ps
endif

###############################################################################

ifndef PNETC
  PNETC = $(SDPA_BIN)/pnetc
endif

ifndef PNET2DOT
  PNET2DOT = $(SDPA_BIN)/pnet2dot
endif

ifndef PNETPUT
  PNETPUT = $(SDPA_BIN)/pnetput
endif

ifndef WE_EXEC
  WE_EXEC = $(SDPA_BIN)/we-eval
endif

ifndef WE_EXEC_WORKER
  WE_EXEC_WORKER = 2
endif

###############################################################################

DEP += Makefile

###############################################################################

PATH_LIB = $(CURDIR)/$(GEN)/pnetc/op
LIB = $(PATH_LIB)/lib$(MAIN).so

###############################################################################

WE_EXEC_LIBPATHS += $(PATH_LIB)

###############################################################################

CXXINCLUDEPATHS += $(SDPA_INCLUDE)

###############################################################################

PNET2DOT_NOINLINE += $(PNET2DOT)
PNET2DOT_NOINLINE += $(addprefix --not-starts-with ,$(NOT_STARTS_WITH))
PNET2DOT_NOINLINE += $(addprefix --not-ends-with ,$(NOT_ENDS_WITH))

PNETC_NOINLINE += $(PNETC)
PNETC_NOINLINE += --no-inline true
PNETC_NOINLINE += --synthesize-virtual-places true

###############################################################################

PNETC += $(addprefix -I,$(SDPA_XML_LIB))
PNETC += --gen-cxxflags=-O3
PNETC += $(addprefix --gen-cxxflags=-I,$(CXXINCLUDEPATHS))
PNETC += $(addprefix --gen-ldflags=-L,$(CXXLIBRARAYPATHS))
PNETC += --force-overwrite-file=true
PNETC += --Woverwrite-file=false
PNETC += --Wbackup-file=false


WE_EXEC += --w $(WE_EXEC_WORKER)
WE_EXEC += $(addprefix --load,$(WE_EXEC_LOAD))
WE_EXEC += $(addprefix -L,$(WE_EXEC_LIBPATHS))
WE_EXEC += -o /dev/null

###############################################################################

.PHONY: default ps all out net put

default: all

ps: $(PS) $(PS_NOINLINE)

net: $(NET)
put: $(PUT)

all: net lib out

###############################################################################

$(NET): $(XML) $(DEP)
	$(PNETC) $(XML) -o $@

$(NET_NOINLINE): $(XML) $(DEP)
	$(PNETC_NOINLINE) $(XML) -o $@

###############################################################################

$(GEN): $(XML) $(DEP)
	$(PNETC) $(XML) -o /dev/null -g $@
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

# does not work for paths that contain spaces
pathify = $(subst $() ,:,$(1))

out: lib $(PUT)
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(call pathify,$(CXXLIBRARAYPATHS)) \
	$(WE_EXEC) --net $(PUT) 2>&1 | $(TEE) $(OUT)

###############################################################################

.PHONY: clean

clean:
	-$(RM) -r $(GEN)
	-$(RM) $(NET)
	-$(RM) $(PUT)
	-$(RM) $(NET_NOINLINE)
	-$(RM) $(PS)
	-$(RM) $(PS_NOINLINE)
	-$(RM) $(OUT)

###############################################################################

.PHONY: help

help:
	@echo "default -> all"
	@echo "all -> net lib out"
	@echo "net -> build pnet $(NET) from $(XML)"
	@echo "lib -> build lib $(LIB) from code in $(GEN)"
	@echo "out -> run $(PUT) with $(WE_EXEC)"
	@echo
	@echo "gen -> generate code into $(GEN)"
	@echo "put -> put tokens into $(NET) and save result in $(PUT)"
	@echo "ps -> generate $(PS) and $(PS_NOINLINE)"
	@echo
	@echo "clean -> delete all generated files"
