
MAIN = n_of_m

SDPA_INCLUDE = $(SDPA_HOME)/include
SDPA_LIBEXEC = $(SDPA_HOME)/libexec
SDPA_XML_LIB = $(SDPA_HOME)/share/sdpa/xml/lib

LIB_DESTDIR = $(HOME)/.sdpa/modules/

PATH_MAIN  = $(CURDIR)

TEE      = /usr/bin/tee
DOT      = /usr/bin/dot -Tps -Grankdir=LR
RM       = /bin/rm -rf
TOUCH    = /usr/bin/touch
CAT      = /bin/cat
CP       = /bin/cp

###############################################################################

XML_MAIN    = $(PATH_MAIN)/n_of_m.xml

###############################################################################

DEP += $(PATH_MAIN)/Makefile

NET = $(PATH_MAIN)/$(MAIN).pnet
GEN = $(PATH_MAIN)/gen
OUT = $(PATH_MAIN)/$(MAIN).out
PS  = $(PATH_MAIN)/$(MAIN).ps

PATH_LIB = $(GEN)/pnetc/op

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

NET_NOINLINE = $(PATH_MAIN)/$(MAIN).noinline.pnet
PS_NOINLINE = $(PATH_MAIN)/$(MAIN).noinline.ps

###############################################################################

WE_EXEC_LOAD += $(SDPA_LIBEXEC)/fake/libfvm-pc.so

WE_EXEC_LIBPATHS += $(PATH_LIB)

###############################################################################

CXXINCLUDEPATHS += $(SDPA_INCLUDE)
CXXINCLUDEPATHS += $(SDPA_INCLUDE)/process

###############################################################################

PNETC    = $(SDPA_HOME)/bin/pnetc
WE_EXEC  = LD_LIBRARY_PATH=$(SDPA_LIBEXEC):${LD_LIBRARY_PATH}
WE_EXEC += FHGLOG_level=MIN
WE_EXEC += FVM_PC_SHMSZ=`echo "$(SHMEM_PER_NODE)" | bc`
WE_EXEC += FVM_PC_FVMSZ=`echo "$(GPI_MEM_PER_NODE)" | bc`
WE_EXEC += $(SDPA_HOME)/bin/we-exec --w 20
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

.PHONY: default ps all out net

default: all

ps: $(PS) $(PS_NOINLINE)

net: $(NET)

all: ps net $(GEN)

###############################################################################

$(NET): $(XML_MAIN) $(DEP)
	$(PNETC) $(XML_MAIN) -o $@

$(NET_NOINLINE): $(XML_MAIN) $(DEP)
	$(PNETC_NOINLINE) $(XML_MAIN) -o $@

###############################################################################

$(GEN): $(XML_MAIN) $(DEP)
	$(PNETC) $(XML_MAIN) -o /dev/null -g $@
	CXXINCLUDEPATHS="$(CXXINCLUDEPATHS)" $(MAKE) -j -C $(GEN)
	$(TOUCH) $@

.PHONY: install

install: $(GEN)
	$(CP) $(LIB) $(LIB_DESTDIR)

###############################################################################

$(PS): $(NET)
	$(PNET2DOT) --input $^ | $(DOT) -o $@

$(PS_NOINLINE): $(NET_NOINLINE)
	$(PNET2DOT_NOINLINE) --input $^ | $(DOT) -o $@

###############################################################################

$(OUT): $(GEN) $(NET)
	$(WE_EXEC) --net $(NET) 2>&1 | $(TEE) $@

out: $(GEN) $(NET)
	$(WE_EXEC) --net $(NET) 2>&1 | $(TEE) $(OUT)

###############################################################################

.PHONY: clean

clean:
	$(RM) $(GEN)
	$(RM) $(NET)
	$(RM) $(NET_NOINLINE)
	$(RM) $(PS)
	$(RM) $(PS_NOINLINE)
	$(RM) $(OUT)
	$(RM) $(XML_GEN)
