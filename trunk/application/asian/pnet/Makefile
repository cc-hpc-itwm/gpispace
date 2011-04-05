
MAIN = asian

TRUNK = $(CURDIR)/../../../
#BUILD = /var/tmp/mr/
BUILD = $(TRUNK)/../build/herc/release
LIB_DESTDIR = $(HOME)/.sdpa/modules

PATH_MAIN  = $(TRUNK)/application/asian/pnet
#UTIL = $(PATH_MAIN)/util
#SEISLIB = $(PATH_MAIN)/seislib

TEE      = /usr/bin/tee
DOT      = /usr/bin/dot -Tps -Grankdir=LR
RM       = /bin/rm -rf
TOUCH    = /usr/bin/touch
CAT      = /bin/cat
CP       = /bin/cp

###############################################################################

XML_MAIN    = xml/wrapper.xml
MK_XML      = $(PATH_MAIN)/mk.xml

XML_GEN += xml/run.xml
XML_GEN += xml/asian.xml

###############################################################################

DEP += Makefile
DEP += xml/wrapper.xml
DEP += xml/collect.xml
DEP += $(XML_GEN)

NET = $(MAIN).pnet
GEN = gen
OUT = $(MAIN).out
PS  = $(MAIN).ps

PATH_LIB = $(PATH_MAIN)/$(GEN)/pnetc/op

LIB = $(PATH_LIB)/lib$(MAIN).so

###############################################################################

%.xml: %.xml.in $(filter-out %.xml,$(DEP))
	$(CAT) $< | $(MK_XML) "$(BUILD)" > $@

###############################################################################

# NOINLINE_NOT_ENDS_WITH += gen

NOINLINE_NOT_STARTS_WITH += asian_id
NOINLINE_NOT_STARTS_WITH += asian_eps1
NOINLINE_NOT_STARTS_WITH += asian_eps2
NOINLINE_NOT_STARTS_WITH += asian_delta
NOINLINE_NOT_STARTS_WITH += collect

NET_NOINLINE = $(MAIN).noinline.pnet
PS_NOINLINE = $(MAIN).noinline.ps

###############################################################################

PNETC_LIB += $(TRUNK)/application/lib

###############################################################################

# WE_EXEC_LOAD += $(BUILD)/fvm-pc/fvm-pc/fake/libfvm-pc.so

WE_EXEC_LIBPATHS += $(PATH_LIB)

LIB_PATH = $(BUILD)/application/kdm:$(BUILD)/application/example/exec

###############################################################################

# CXXINCLUDEPATHS += $(TRUNK)/fvm-pc/trunk
CXXINCLUDEPATHS += $(TRUNK)/fhglog/trunk
CXXINCLUDEPATHS += $(TRUNK)/application/example/exec
CXXINCLUDEPATHS += $(BUILD)/fhglog

# determine_size, do_load, do_write
# CXXINCLUDEPATHS += $(TRUNK)/application/kdm/src

# CXXINCLUDEPATHS += $(UTIL)
# CXXINCLUDEPATHS += $(SEISLIB)

###############################################################################

PNETC    = $(BUILD)/xml/xml/parse/pnetc
WE_EXEC  = LD_LIBRARY_PATH=$(LIB_PATH):${LD_LIBRARY_PATH}
WE_EXEC += FHGLOG_level=MIN
# WE_EXEC += FVM_PC_SHMSZ=`echo "$(SHMEM_PER_NODE)" | bc`
# WE_EXEC += FVM_PC_FVMSZ=`echo "$(GPI_MEM_PER_NODE)" | bc`
WE_EXEC += $(BUILD)/we/apps/we-exec --w 1
PNET2DOT = $(BUILD)/we/apps/pnet2dot

###############################################################################

PNETC += $(addprefix -I ,$(PNETC_LIB))

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

default: $(OUT)

ps: $(PS) $(PS_NOINLINE)

net: $(NET)

all: ps default

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
