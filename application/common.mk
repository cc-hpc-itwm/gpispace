
ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

###############################################################################

SHELL = /bin/sh
MAKEFLAGS += -r

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

ifndef PNETC
  PNETC = $(SDPA_BIN)/pnetc
endif

ifndef PNET2DOT
  PNET2DOT = $(SDPA_BIN)/pnet2dot
endif

ifndef PNETPUT
  PNETPUT = $(SDPA_BIN)/pnetput
endif

ifndef PNETV
  PNETV = $(SDPA_BIN)/pnetv
endif

ifndef WE_EXEC_CMD
  WE_EXEC_CMD = $(SDPA_BIN)/we-exec
endif

ifndef WE_EXEC_WORKER
  WE_EXEC_WORKER = 2
endif

###############################################################################

ifndef XML
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable XML)
  else
    XML = $(CURDIR)/$(MAIN).xpnet
  endif
endif

ifndef NET
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable NET)
  else
    NET = $(CURDIR)/$(MAIN).pnet
  endif
endif

ifndef NET_NOINLINE
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable NET_NOINLINE)
  else
    NET_NOINLINE = $(CURDIR)/$(MAIN).noinline.pnet
  endif
endif

ifndef NET_VERIFICATION
  ifndef NET
    $(error variable NET undefined but needed to derive variable NET_VERIFICATION)
  else
    NET_VERIFICATION = $(NET).verification
  endif
endif

ifndef PUT
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable PUT)
  else
    PUT = $(NET).put
  endif
endif

ifndef GEN
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable GEN)
  else
    GEN = $(CURDIR)/gen
  endif
endif

ifndef OUT
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable OUT)
  else
    OUT = $(CURDIR)/$(MAIN).out
  endif
endif

ifndef PS
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable PS)
  else
    PS = $(CURDIR)/$(MAIN).ps
  endif
endif

ifndef PS_NOINLINE
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable PS_NOINLINE)
  else
    PS_NOINLINE = $(CURDIR)/$(MAIN).noinline.ps
  endif
endif

###############################################################################

DEP += $(CURDIR)/Makefile
PATH_LIB = $(GEN)/pnetc/op
WE_EXEC_LIBPATHS += $(PATH_LIB)
CXXINCLUDEPATHS += $(SDPA_INCLUDE)

DEP_XML = $(XML).d

###############################################################################

PNET2DOT += $(addprefix --not-starts-with ,$(NOT_STARTS_WITH))
PNET2DOT += $(addprefix --not-ends-with ,$(NOT_ENDS_WITH))

PNETC_NOINLINE += $(PNETC)
PNETC_NOINLINE += --no-inline true
PNETC_NOINLINE += --synthesize-virtual-places true

PNETC += $(addprefix -I,$(SDPA_XML_LIB))
PNETC += --gen-cxxflags=-O3
PNETC += $(addprefix --gen-cxxflags=-I,$(CXXINCLUDEPATHS))
PNETC += $(addprefix --gen-ldflags=-L,$(CXXLIBRARYPATHS))
PNETC += --force-overwrite-file=true

# does not work for paths that contain spaces
pathify = $(subst $() ,:,$(1))

WE_EXEC = LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(call pathify,$(CXXLIBRARYPATHS))
WE_EXEC += $(WE_EXEC_ENV)
WE_EXEC += $(WE_EXEC_CMD)
WE_EXEC += --w $(WE_EXEC_WORKER)
WE_EXEC += $(addprefix --load ,$(WE_EXEC_LOAD))
WE_EXEC += $(addprefix -L,$(WE_EXEC_LIBPATHS))
WE_EXEC += -o /dev/null

###############################################################################

.PHONY: default ps net verify put gen lib run

default: run

ps: $(PS) $(PS_NOINLINE)

net: $(NET)
put: $(PUT)
gen: $(GEN)
verify: $(NET_VERIFICATION)

###############################################################################

$(DEP_XML): $(XML)
	$(PNETC) -i $(XML) -o /dev/null $(PNETC_OPTS) -MM -MT '$(DEP_XML)' > $@

ifneq ($(wildcard $(DEP_XML)),)
  include $(DEP_XML)
endif

###############################################################################

$(NET): $(DEP_XML) $(XML) $(DEP)
	$(PNETC) -i $(XML) $(PNETC_OPTS) -o $@

$(NET_NOINLINE): $(DEP_XML) $(XML) $(DEP)
	$(PNETC_NOINLINE) -i $(XML) $(PNETC_OPTS) -o $@

$(PUT): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUT) $(addprefix -p ,$(PUT_PORT))

###############################################################################

$(NET_VERIFICATION): $(NET)
	$(PNETV) -i $(NET) > $@

###############################################################################

$(GEN): $(DEP_XML) $(XML) $(DEP)
	$(PNETC) -i $(XML) -o /dev/null $(PNETC_OPTS) -g $@
	$(TOUCH) $@

lib: $(GEN)
	$(MAKE) -C $(GEN)

###############################################################################

$(PS): $(NET)
	$(PNET2DOT) --input $^ | $(DOT) -o $@

$(PS_NOINLINE): $(NET_NOINLINE)
	$(PNET2DOT) --input $^ | $(DOT) -o $@

###############################################################################

run: lib $(PUT)
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
	-$(RM) $(DEP_XML)
	-$(RM) $(NET_VERIFICATION)
	-$(RM) *~

###############################################################################

.PHONY: help

help:
	@echo "default     'run'"
	@echo
	@echo "net         build pnet from xml"
	@echo "put         'net' & put tokens into the workflow"
	@echo
	@echo "gen         generate code into gen"
	@echo "lib         'gen' & build libs from code in gen"
	@echo "run         'lib' & 'put' & execute workflow"
	@echo
	@echo "verify      'net' & verify the pnet"
	@echo "ps          'net' & generate postscript"
	@echo
	@echo "clean       delete all generated files"
	@echo
	@echo "showconfig  show the actual configuration"

###############################################################################

.PHONY: showconfig

showconfig:
	@echo "MAIN = $(MAIN)"
	@echo
	@echo "SDPA_HOME = $(SDPA_HOME)"
	@echo
	@echo "TEE = $(TEE)"
	@echo "DOT = $(DOT)"
	@echo "RM = $(RM)"
	@echo "TOUCH = $(TOUCH)"
	@echo
	@echo "SDPA_INCLUDE = $(SDPA_INCLUDE)"
	@echo "SDPA_BIN = $(SDPA_BIN)"
	@echo "SDPA_XML_LIB = $(SDPA_XML_LIB)"
	@echo
	@echo "XML = $(XML)"
	@echo "NET = $(NET)"
	@echo "NET_NOINLINE = $(NET_NOINLINE)"
	@echo "NET_VERIFICATION = $(NET_VERIFICATION)"
	@echo "PUT = $(PUT)"
	@echo "GEN = $(GEN)"
	@echo "OUT = $(OUT)"
	@echo "PS = $(PS)"
	@echo "PS_NOINLINE = $(PS_NOINLINE)"
	@echo
	@echo "DEP = $(DEP)"
	@echo "PATH_LIB = $(PATH_LIB)"
	@echo "WE_EXEC_LIBPATHS = $(WE_EXEC_LIBPATHS)"
	@echo
	@echo "CXXINCLUDEPATHS = $(CXXINCLUDEPATHS)"
	@echo "CXXLIBRARYPATHS = $(CXXLIBRARYPATHS)"
	@echo
	@echo "PNETC = $(PNETC)"
	@echo "PNETC_NOINLINE = $(PNETC_NOINLINE)"
	@echo "PNET2DOT = $(PNET2DOT)"
	@echo "PNETPUT = $(PNETPUT)"
	@echo "WE_EXEC = $(WE_EXEC)"
