
ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

###############################################################################

SHELL = /bin/sh
MAKEFLAGS += -r

###############################################################################

ifndef TEE
  TEE = $(shell which tee 2>/dev/null)
endif

ifndef CMD_DOT
  CMD_DOT = $(shell which dot 2>/dev/null)
endif

ifndef RM
  RM = $(shell which rm 2>/dev/null)
endif

ifndef TOUCH
  TOUCH = $(shell which touch 2>/dev/null)
endif

ifndef XMLLINT
  XMLLINT = $(shell which xmllint 2>/dev/null)
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

ifndef SDPA_LIBEXEC
  SDPA_LIBEXEC = $(SDPA_HOME)/libexec
endif

ifndef SDPA_XML_SCHEMA
  SDPA_XML_SCHEMA = $(SDPA_HOME)/share/sdpa/xml/xsd/pnet.xsd
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

ifndef WE_EXEC_OUTPUT
  WE_EXEC_OUTPUT = /dev/null
endif

ifndef SDPA
  SDPA = $(SDPA_BIN)/sdpa
endif

###############################################################################

ifndef XML
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable XML)
  else
    XML = $(CURDIR)/$(MAIN).xpnet
  endif
endif

ifndef DEP_XML
  ifndef XML
    $(error variable XML undefined but needed to derive variable DEP_XML)
  else
    DEP_XML = $(XML).d
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

ifndef NET_VALIDATION
  ifndef XML
    $(error variable XML undefined but needed to derive variable NET_VALIDATION)
  else
    NET_VALIDATION = $(NET).validation
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

ifndef DOT
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable DOT)
  else
    DOT = $(CURDIR)/$(MAIN).dot
  endif
endif

ifndef DOT_NOINLINE
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable DOT_NOINLINE)
  else
    DOT_NOINLINE = $(CURDIR)/$(MAIN).noinline.dot
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

ifndef SVG
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable SVG)
  else
    SVG = $(CURDIR)/$(MAIN).svg
  endif
endif

ifndef SVG_NOINLINE
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable SVG_NOINLINE)
  else
    SVG_NOINLINE = $(CURDIR)/$(MAIN).noinline.svg
  endif
endif

ifndef DESTDIR
  ifndef MAIN
    $(error variable MAIN undefined but needed to derive variable DESTDIR)
  else
    DESTDIR = $(SDPA_LIBEXEC)/apps/$(MAIN)
  endif
endif

ifndef LIB_DESTDIR
  ifndef DESTDIR
    $(error variable DESTDIR undefined but needed to derive variable LIB_DESTDIR)
  else
    LIB_DESTDIR = $(DESTDIR)/modules
  endif
endif

###############################################################################

PATH_LIB += $(GEN)/pnetc/op
WE_EXEC_LIBPATHS += $(PATH_LIB)
CXXINCLUDEPATHS += $(SDPA_INCLUDE)

###############################################################################

PNET2DOT += $(addprefix --not-starts-with ,$(NOT_STARTS_WITH))
PNET2DOT += $(addprefix --not-ends-with ,$(NOT_ENDS_WITH))
PNET2DOT += $(PNET2DOT_OPTS)

PNETC += $(addprefix -I,$(SDPA_XML_LIB))
PNETC += --gen-cxxflags=-O3
PNETC += $(addprefix --gen-cxxflags=-I,$(CXXINCLUDEPATHS))
PNETC += $(addprefix --gen-ldflags=-L,$(CXXLIBRARYPATHS))
PNETC += --force-overwrite-file=true
PNETC += $(PNETC_OPTS)

PNETC_NOINLINE += $(PNETC)
PNETC_NOINLINE += --no-inline true
PNETC_NOINLINE += --synthesize-virtual-places true

PNETC_LIST_DEPENDENCIES = $(PNETC) --list-dependencies /dev/stdout

PNETPUT += $(addprefix -p ,$(PUT_PORT))
PNETPUT += $(PNETPUT_OPTS)

PNETV += $(PNETV_OPTS)

# does not work for paths that contain spaces
pathify = $(subst $() ,:,$(1))

ifneq "$(CXXLIBRARYPATHS)" ""
  WE_EXEC = LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(call pathify,$(CXXLIBRARYPATHS))
endif
WE_EXEC += $(WE_EXEC_ENV)
WE_EXEC += $(WE_EXEC_CMD)
WE_EXEC += --w $(WE_EXEC_WORKER)
WE_EXEC += $(addprefix --load ,$(WE_EXEC_LOAD))
WE_EXEC += $(addprefix -L,$(WE_EXEC_LIBPATHS))
WE_EXEC += -o $(WE_EXEC_OUTPUT)
WE_EXEC += $(WE_EXEC_OPTS)

XMLLINT += --noout
XMLLINT += --schema $(SDPA_XML_SCHEMA)

###############################################################################

.PHONY: default build dot ps svg net verify validate put gen lib run submit

default: build

build: put lib $(BUILD)

dot: $(DOT) $(DOT_NOINLINE)
ps: $(PS) $(PS_NOINLINE)
svg: $(SVG) $(SVG_NOINLINE)
net: $(NET)
put: $(PUT)
gen: $(GEN)
verify: $(NET_VERIFICATION)
validate: $(NET_VALIDATION)

###############################################################################

$(DEP_XML): $(XML)
	$(PNETC) -i $(XML) -o /dev/null -MP -MT '$(DEP_XML)' -MM $@

ifneq "$(wildcard $(DEP_XML))" ""
  include $(DEP_XML)
endif

###############################################################################

$(NET): $(DEP_XML) $(XML) $(DEP)
	$(PNETC) -i $(XML) -o $@

$(NET_NOINLINE): $(DEP_XML) $(XML) $(DEP)
	$(PNETC_NOINLINE) -i $(XML) -o $@

$(PUT): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUT)

###############################################################################

$(NET_VERIFICATION): $(NET)
	$(PNETV) -i $(NET) > $@

###############################################################################

ifeq "$(XMLLINT)" ""

$(NET_VALIDATION):
	$(error Cannot validate: Missing 'xmllint'.)

else

ifeq "$(TEE)" ""

$(NET_VALIDATION): $(DEP_XML) $(XML) $(DEP)
	$(XMLLINT) $$($(PNETC_LIST_DEPENDENCIES) -i $(XML) -o /dev/null) 2> "$@"

else

$(NET_VALIDATION): $(DEP_XML) $(XML) $(DEP)
	$(XMLLINT) $$($(PNETC_LIST_DEPENDENCIES) -i $(XML) -o /dev/null) 2>&1 | $(TEE) "$@"

endif
endif

###############################################################################

ifeq "$(TOUCH)" ""

$(GEN): $(DEP_XML) $(XML) $(DEP)
	$(warning Missing 'touch'. Most probably some timestamps will be wrong.)
	$(PNETC) -i $(XML) -o /dev/null -g $@

else

$(GEN): $(DEP_XML) $(XML) $(DEP)
	$(PNETC) -i $(XML) -o /dev/null -g $@
	$(TOUCH) $@

endif

lib: $(GEN)
	$(MAKE) -C $(GEN)

###############################################################################

$(DOT): $(NET)
	$(PNET2DOT) --input $^ --output $@

$(DOT_NOINLINE): $(NET_NOINLINE)
	$(PNET2DOT) --input $^ --output $@

ifeq "$(CMD_DOT)" ""

$(PS) $(PS_NOINLINE) $(SVG) $(SVG_NOINLINE):
	$(error Cannot create $@: Missing 'dot'.)

else

$(PS): $(DOT)
	$(CMD_DOT) $^ -Tps -o $@

$(SVG): $(DOT)
	$(CMD_DOT) $^ -Tsvg -o $@

$(PS_NOINLINE): $(DOT_NOINLINE)
	$(CMD_DOT) $^ -Tps -o $@

$(SVG_NOINLINE): $(DOT_NOINLINE)
	$(CMD_DOT) $^ -Tsvg -o $@

endif

###############################################################################

ifeq "$(TEE)" ""

run: lib $(PUT)
	$(warning Missing 'tee'. Save output into $(OUT).)
	$(warning To watch the output on the fly install 'tee'.)
	$(WE_EXEC) --net $(PUT) 2>&1 > $(OUT)

else

run: lib $(PUT)
	$(WE_EXEC) --net $(PUT) 2>&1 | $(TEE) $(OUT)

endif

###############################################################################

submit: $(PUT)
	$(SDPA) submit $(PUT) $(OUT)

###############################################################################

.PHONY: install modinstall

install: modinstall

modinstall: lib
	$(MAKE) LIB_DESTDIR=$(LIB_DESTDIR) -C $(GEN) install

###############################################################################

.PHONY: clean

ifeq "$(RM)" ""

clean:
	$(error Cannot clean: Missing 'rm'.)

else

clean: $(CLEAN)
	-$(RM) -f -r $(GEN)
	-$(RM) -f $(NET)
	-$(RM) -f $(PUT)
	-$(RM) -f $(NET_NOINLINE)
	-$(RM) -f $(DOT)
	-$(RM) -f $(DOT_NOINLINE)
	-$(RM) -f $(PS)
	-$(RM) -f $(PS_NOINLINE)
	-$(RM) -f $(SVG)
	-$(RM) -f $(SVG_NOINLINE)
	-$(RM) -f $(OUT)
	-$(RM) -f $(DEP_XML)
	-$(RM) -f $(NET_VERIFICATION)
	-$(RM) -f $(NET_VALIDATION)
	-$(RM) -f *~

endif

###############################################################################

.PHONY: help

help:
	@echo "default     'build'"
	@echo
	@echo "build       'put' & 'lib'"
	@echo
	@echo "net         build pnet from xml"
	@echo "put         'net' & put tokens into the workflow"
	@echo
	@echo "gen         generate code into gen"
	@echo "lib         'gen' & build libs from code in gen"
	@echo "run         'lib' & 'put' & execute workflow"
	@echo "submit      'put' & submit to a running SDPA system"
	@echo
	@echo "validate    validate the xml"
	@echo "verify      'net' & verify the pnet"
	@echo
	@echo "dot         'net' & generate .dot"
	@echo "ps          'dot' & generate postscript"
	@echo "svg         'dot' & generate svg"
	@echo
	@echo "clean       delete all generated files"
	@echo
	@echo "showconfig  show the actual configuration"
	@echo
	@echo "modinstall  'lib' & install module(s) to $(LIB_DESTDIR)"
	@echo
	@echo "install     'modinstall'"

###############################################################################

.PHONY: showconfig

showconfig:
	@echo "*** Needed parameters:"
	@echo
	@echo "SDPA_HOME = $(SDPA_HOME)"
	@echo
	@echo "MAIN = $(MAIN)"
	@echo
	@echo "*** External programs:"
	@echo
	@echo "TEE     = $(TEE)"
	@echo "CMD_DOT = $(CMD_DOT)"
	@echo "RM      = $(RM)"
	@echo "TOUCH   = $(TOUCH)"
	@echo "XMLLINT = $(XMLLINT)"
	@echo
	@echo "*** GPI-Space paths and files:"
	@echo
	@echo "DESTDIR         = $(DESTDIR)"
	@echo "SDPA_INCLUDE    = $(SDPA_INCLUDE)"
	@echo "SDPA_BIN        = $(SDPA_BIN)"
	@echo "SDPA_XML_LIB    = $(SDPA_XML_LIB)"
	@echo "SDPA_LIBEXEC    = $(SDPA_LIBEXEC)"
	@echo
	@echo "SDPA_XML_SCHEMA = $(SDPA_XML_SCHEMA)"
	@echo
	@echo "*** Files:"
	@echo
	@echo "XML              = $(XML)"
	@echo "DEP_XML          = $(DEP_XML)"
	@echo "NET              = $(NET)"
	@echo "NET_NOINLINE     = $(NET_NOINLINE)"
	@echo "NET_VERIFICATION = $(NET_VERIFICATION)"
	@echo "NET_VALIDATION   = $(NET_VALIDATION)"
	@echo "PUT              = $(PUT)"
	@echo "GEN              = $(GEN)"
	@echo "OUT              = $(OUT)"
	@echo "DOT              = $(DOT)"
	@echo "DOT_NOINLINE     = $(DOT_NOINLINE)"
	@echo "PS               = $(PS)"
	@echo "PS_NOINLINE      = $(PS_NOINLINE)"
	@echo "SVG              = $(SVG)"
	@echo "SVG_NOINLINE     = $(SVG_NOINLINE)"
	@echo
	@echo "*** Dependencies and options:"
	@echo
	@echo "DEP              = $(DEP)"
	@echo "PATH_LIB         = $(PATH_LIB)"
	@echo "WE_EXEC_LIBPATHS = $(WE_EXEC_LIBPATHS)"
	@echo "LIB_DESTDIR      = $(LIB_DESTDIR)"
	@echo
	@echo "CXXINCLUDEPATHS  = $(CXXINCLUDEPATHS)"
	@echo "CXXLIBRARYPATHS  = $(CXXLIBRARYPATHS)"
	@echo
	@echo "NOT_STARTS_WITH  = $(NOT_STARTS_WITH)"
	@echo "NOT_ENDS_WITH    = $(NOT_ENDS_WITH)"
	@echo "PNET2DOT_OPTS    = $(PNET2DOT_OPTS)"
	@echo
	@echo "PNETC_OPTS       = $(PNETC_OPTS)"
	@echo
	@echo "PUT_PORT         = $(PUT_PORT)"
	@echo "PNETPUT_OPTS     = $(PNETPUT_OPTS)"
	@echo
	@echo "PNETV_OPTS       = $(PNETV_OPTS)"
	@echo
	@echo "WE_EXEC_ENV      = $(WE_EXEC_ENV)"
	@echo "WE_EXEC_CMD      = $(WE_EXEC_CMD)"
	@echo "WE_EXEC_OUTPUT   = $(WE_EXEC_OUTPUT)"
	@echo "WE_EXEC_WORKER   = $(WE_EXEC_WORKER)"
	@echo "WE_EXEC_LOAD     = $(WE_EXEC_LOAD)"
	@echo "WE_EXEC_LIBPATHS = $(WE_EXEC_LIBPATHS)"
	@echo "WE_EXEC_OPTS     = $(WE_EXEC_OPTS)"
	@echo
	@echo "BUILD            = $(BUILD)"
	@echo "CLEAN            = $(CLEAN)"
	@echo
	@echo "*** Derived commands:"
	@echo
	@echo "PNETC                   = $(PNETC)"
	@echo "PNETC_NOINLINE          = $(PNETC_NOINLINE)"
	@echo "PNETC_LIST_DEPENDENCIES = $(PNETC_LIST_DEPENDENCIES)"
	@echo "PNET2DOT                = $(PNET2DOT)"
	@echo "PNETPUT                 = $(PNETPUT)"
	@echo "PNETV                   = $(PNETV)"
	@echo "WE_EXEC                 = $(WE_EXEC)"
	@echo "SDPA                    = $(SDPA)"
	@echo "XMLLINT                 = $(XMLLINT)"
