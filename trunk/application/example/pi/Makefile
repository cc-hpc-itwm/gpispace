
MAIN = pi

TRUNK = $(CURDIR)/../../../
BUILD = $(TRUNK)/../build/gcc43

#include ../make.defs

PATH_MAIN  = $(TRUNK)/application/example/$(MAIN)

TEE      = /usr/bin/tee
DOT      = /usr/bin/dot -Tps -Grankdir=LR
RM       = /bin/rm -rf
TOUCH    = /usr/bin/touch
CAT      = /bin/cat
CP       = /bin/cp

###############################################################################

XML_MAIN    = xml/$(MAIN).xml
XML_MAIN_IN = $(XML_MAIN).in
MK_XML      = $(TRUNK)/application/example/$(MAIN)/mk.xml

###############################################################################

DEP += Makefile

NET = $(MAIN).pnet
GEN = gen
OUT = $(MAIN).out
PS  = $(MAIN).ps

PATH_LIB = $(PATH_MAIN)/$(GEN)/pnetc/op

LIB = $(PATH_LIB)/lib$(MAIN).so

###############################################################################

NUM_PACKET = 100
POINTS_PER_PACKET = 4 * 10^6
CREDIT = 8
SEED = 271828

$(XML_MAIN): $(XML_MAIN_IN) $(DEP)
	$(CAT) $< | $(MK_XML) "$(NUM_PACKET)" "$(POINTS_PER_PACKET)" "$(CREDIT)" "$(SEED)" > $@

###############################################################################

#NOINLINE_NOT_ENDS_WITH += generate

#NOINLINE_NOT_STARTS_WITH += dup

NET_NOINLINE = $(MAIN).noinline.pnet
PS_NOINLINE = $(MAIN).noinline.ps

###############################################################################

PNETC_LIB += $(TRUNK)/application/lib

###############################################################################

#WE_EXEC_LOAD += $(BUILD)/fvm-pc/fvm-pc/fake/libfvm-pc.so

WE_EXEC_LIBPATHS += $(TRUNK)/application/example/$(MAIN)/$(GEN)/pnetc/op

#LIB_PATH = $(BUILD)/application/kdm:$(BUILD)/application/example/exec

###############################################################################

#CXXINCLUDEPATHS += $(TRUNK)/fvm-pc/trunk
CXXINCLUDEPATHS += $(TRUNK)/fhglog/trunk
#CXXINCLUDEPATHS += $(TRUNK)/application/example/exec
CXXINCLUDEPATHS += $(BUILD)/fhglog

# determine_size, do_load, do_write
#CXXINCLUDEPATHS += $(TRUNK)/application/kdm/src

# print.hpp, comm.hpp
#CXXINCLUDEPATHS += $(UTIL)

###############################################################################

PNETC    = $(BUILD)/xml/xml/parse/pnetc
WE_EXEC  = LD_LIBRARY_PATH=$(LIB_PATH):${LD_LIBRARY_PATH} $(BUILD)/we/apps/we-exec --w $(CREDIT)
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
	$(CP) $(LIB) $(SDPA_LIBEXEC)

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
	$(RM) $(XML_MAIN)
