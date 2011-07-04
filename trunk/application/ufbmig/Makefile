
MAIN = ufbmig

NETS += init
NETS += finalize

SDPA_INCLUDE = $(SDPA_HOME)/include
SDPA_LIBEXEC = $(SDPA_HOME)/libexec
SDPA_XML_LIB = $(SDPA_HOME)/share/sdpa/xml/lib

LIB_DESTDIR = $(HOME)/.sdpa/modules/

###############################################################################

PATH_MAIN  = $(CURDIR)
PATH_XML   = $(PATH_MAIN)/xml
PATH_GEN   = $(PATH_MAIN)/gen
PATH_LIB   = $(PATH_GEN)/pnetc/op
PATH_UTIL  = $(PATH_MAIN)/util

###############################################################################

pathsearch = $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(PATH)))))

TEE   := $(call pathsearch,tee)
DOT   := $(call pathsearch,dot) -Tps -Grankdir=LR
RM    := $(call pathsearch,rm) -rf
TOUCH := $(call pathsearch,touch)
CAT   := $(call pathsearch,cat)
CP    := $(call pathsearch,cp)

###############################################################################

DEP += $(PATH_MAIN)/Makefile
DEP += $(PATH_XML)/types.xml
DEP += $(PATH_UTIL)/memory.hpp

LIB = $(PATH_LIB)/lib$(MAIN).so

###############################################################################

SHMEM_PER_NODE = 2^28
GPI_MEM_PER_NODE = 2^28

###############################################################################

NOINLINE_NOT_STARTS_WITH += dup
NOINLINE_NOT_STARTS_WITH += scatter
NOINLINE_NOT_STARTS_WITH += generate
NOINLINE_NOT_STARTS_WITH += wait
NOINLINE_NOT_STARTS_WITH += eat

###############################################################################

WE_EXEC_LOAD += $(SDPA_LIBEXEC)/fake/libfvm-pc.so

WE_EXEC_LIBPATHS += $(PATH_LIB)

###############################################################################

CXXINCLUDEPATHS += $(SDPA_INCLUDE)
CXXINCLUDEPATHS += $(SDPA_INCLUDE)/process

CXXINCLUDEPATHS += $(PATH_MAIN)

###############################################################################

PNETC    = $(SDPA_HOME)/bin/pnetc
WE_EXEC  = LD_LIBRARY_PATH=$(SDPA_LIBEXEC):${LD_LIBRARY_PATH}
WE_EXEC += FHGLOG_level=MIN
WE_EXEC += FVM_PC_SHMSZ=`echo "$(SHMEM_PER_NODE)" | bc`
WE_EXEC += FVM_PC_FVMSZ=`echo "$(GPI_MEM_PER_NODE)" | bc`
WE_EXEC += $(SDPA_HOME)/bin/we-exec --w 2
PNET2DOT = $(SDPA_HOME)/bin/pnet2dot

###############################################################################

PNETC += $(addprefix -I ,$(SDPA_XML_LIB))

WE_EXEC += $(addprefix --load ,$(WE_EXEC_LOAD))
WE_EXEC += $(addprefix -L ,$(WE_EXEC_LIBPATHS))

###############################################################################

PNET2DOT_NOINLINE += $(PNET2DOT)
PNET2DOT_NOINLINE += $(addprefix --not-starts-with ,$(NOINLINE_NOT_STARTS_WITH))
PNET2DOT_NOINLINE += $(addprefix --not-ends-with ,$(NOINLINE_NOT_ENDS_WITH))

PNETC_NOINLINE += $(PNETC)
PNETC_NOINLINE += --no-inline true
PNETC_NOINLINE += --synthesize-virtual-places true
PNETC_NOINLINE += --Onot true

###############################################################################

PNET          = $(addsuffix .pnet, $(NETS))
NOINLINE_PNET = $(addsuffix .noinline.pnet, $(NETS))

PS            = $(addsuffix .ps, $(NETS))
NOINLINE_PS   = $(addsuffix .noinline.ps, $(NETS))

xml           = $(PATH_XML)/$(1).xml

###############################################################################

.PHONY: default nets net noinline.net

default: nets pss

net: $(PNET)
noinline.net: $(NOINLINE_PNET)
nets: net noinline.net

ps: $(PS)
noinline.ps: $(NOINLINE_PS)
pss: ps noinline.ps

###############################################################################

%.pnet: $(call xml,%) $(DEP)
	$(PNETC) $< -o $@

%.noinline.pnet: $(call xml,%) $(DEP)
	$(PNETC_NOINLINE) $< -o $@

###############################################################################

%.ps: %.pnet
	$(PNET2DOT) --input $< | $(DOT) -o $@

###############################################################################

.PHONY: gen

gen: $(PATH_GEN)

PNETC_GEN  = $(PNETC)
PNETC_GEN += -o /dev/null
PNETC_GEN += -g $(PATH_GEN)
PNETC_GEN += --Windependent-transition false
PNETC_GEN += --Woverwrite-file false

GEN_HEADER = '<defun name="all"><net>'
GEN_FOOTER = '</net></defun>'
GEN_TRANS = '<transition name="$(1)">                      \
               <include-function href="$(call xml,$(1))"/> \
             </transition>'

$(PATH_GEN): $(foreach n,$(NETS),$(call xml,$(n))) $(DEP)
	echo $(GEN_HEADER) \
              $(foreach n,$(NETS),$(call GEN_TRANS,$(n))) \
              $(GEN_FOOTER) | $(PNETC_GEN)
	CXXINCLUDEPATHS="$(CXXINCLUDEPATHS)" $(MAKE) -j -C $(PATH_GEN)
	$(TOUCH) $@

###############################################################################

.PHONY: install

install: $(PATH_GEN)
	$(CP) $(LIB) $(LIB_DESTDIR)

###############################################################################

%.run.init: $(GEN) init.pnet
	pnetput --if init.pnet --of init.putted.pnet -p description=\"$*\"
	$(WE_EXEC) --net init.putted.pnet

$(OUT): $(GEN) $(NET)
	$(WE_EXEC) --net $(NET) 2>&1 | $(TEE) $@

out: $(GEN) $(NET)
	$(WE_EXEC) --net $(NET) 2>&1 | $(TEE) $(OUT)

###############################################################################

.PHONY: clean

clean:
	$(RM) $(PNET)
	$(RM) $(NOINLINE_PNET)
	$(RM) $(PS)
	$(RM) $(NOINLINE_PS)
	$(RM) $(PATH_GEN)

###############################################################################

.PHONY: show_config

show_config:
	@echo
	@echo "Variables:"
	@echo "PATH_MAIN: $(PATH_MAIN)"
	@echo
	@echo "Programs:"
	@echo "TEE:   $(TEE)"
	@echo "DOT:   $(DOT)"
	@echo "RM:    $(RM)"
	@echo "TOUCH: $(TOUCH)"
	@echo "CAT:   $(CAT)"
	@echo "CP:    $(CP)"
