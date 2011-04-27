
MAIN = sp

TRUNK = $(CURDIR)/../../../
#BUILD = /var/tmp/mr/
BUILD = $(TRUNK)/../build/herc/release
LIB_DESTDIR = $(HOME)/.sdpa/modules

PATH_MAIN  = $(TRUNK)/application/suowmg/sp
UTIL = $(PATH_MAIN)/util
SEISLIB = $(PATH_MAIN)/seislib

TEE      = /usr/bin/tee
DOT      = /usr/bin/dot -Tps -Grankdir=LR
RM       = /bin/rm -rf
TOUCH    = /usr/bin/touch
CAT      = /bin/cat
CP       = /bin/cp

###############################################################################

XML_MAIN    = xml/$(MAIN).xml
MK_XML      = $(PATH_MAIN)/mk.xml

XML_GEN += $(XML_MAIN)
XML_GEN += xml/load.xml
XML_GEN += xml/run.xml
XML_GEN += xml/write.xml

###############################################################################

DEP += xml/generate/package.xml
DEP += xml/generate/interval.xml
DEP += xml/interval/merge.xml
DEP += xml/interval/back.xml
DEP += xml/join.xml
DEP += $(UTIL)/print.hpp
DEP += $(UTIL)/comm.hpp
DEP += Makefile
DEP += $(XML_GEN)

NET = $(MAIN).pnet
GEN = gen
OUT = $(MAIN).out
PS  = $(MAIN).ps

PATH_LIB = $(PATH_MAIN)/$(GEN)/pnetc/op

LIB = $(PATH_LIB)/lib$(MAIN).so

###############################################################################

FILE_INPUT       = /fhgfs/HPC/rahn/suowmg/spike6.tmax=5.0.shot8.su
#FILE_INPUT       = /fhgfs/HPC/rahn/suowmg/shot64.su
TYPE_INPUT       = su
FILE_OUTPUT      = /fhgfs/HPC/rahn/suowmg/out.shot8.su
#FILE_OUTPUT      = /fhgfs/HPC/rahn/suowmg/shot64.out.su
TYPE_OUTPUT      = su
SHMEM_PER_NODE   = 6  * 1824 +     167233040
GPI_MEM_PER_NODE = 18 * 1824 + 2 * 167236640
#SHMEM_PER_NODE   = 1073741824
#GPI_MEM_PER_NODE = 2 * 1073741824

NX = 1751
NY = 2001
NZ = 751
DX = 20.0
DY = 20.0
DZ = 20.0
VPFILE = /fhgfs/HPC/rahn/suowmg/model.bin
EFILE = none
DFILE = none
ZMAX = 4000.0
LX = 8000.0
LY = 8000.0
F1 = 4.0
F2 = 5.0
F3 = 8.0
F4 = 12.0
PAD = 0.0
LATSAMPLESPERWAVE = 3.5
VERTSAMPLESPERWAVE = 3.5
MEDIUM = ISO
PROPAGATOR = FFD3

%.xml: %.xml.in $(filter-out %.xml,$(DEP))
	$(CAT) $< | $(MK_XML) "$(FILE_INPUT)" \
                              "$(TYPE_INPUT)" \
                              "$(FILE_OUTPUT)" \
                              "$(TYPE_OUTPUT)" \
                              "$(SHMEM_PER_NODE)" \
                              "$(GPI_MEM_PER_NODE)" \
                              "$(BUILD)" \
                              "$(NX)" \
                              "$(NY)" \
                              "$(NZ)" \
                              "$(DX)" \
                              "$(DY)" \
                              "$(DZ)" \
                              "$(VPFILE)" \
                              "$(EFILE)" \
                              "$(DFILE)" \
                              "$(ZMAX)" \
                              "$(LX)" \
                              "$(LY)" \
                              "$(F1)" \
                              "$(F2)" \
                              "$(F3)" \
                              "$(F4)" \
                              "$(PAD)" \
                              "$(LATSAMPLESPERWAVE)" \
                              "$(VERTSAMPLESPERWAVE)" \
                              "$(MEDIUM)" \
                              "$(PROPAGATOR)" \
                              > $@

###############################################################################

NOINLINE_NOT_ENDS_WITH += gen

NOINLINE_NOT_STARTS_WITH += dup
NOINLINE_NOT_STARTS_WITH += scatter
NOINLINE_NOT_STARTS_WITH += generate_interval
NOINLINE_NOT_STARTS_WITH += generate_package
NOINLINE_NOT_STARTS_WITH += generate_output_slot
NOINLINE_NOT_STARTS_WITH += wait
NOINLINE_NOT_STARTS_WITH += eat
NOINLINE_NOT_STARTS_WITH += merge_interval
NOINLINE_NOT_STARTS_WITH += join_shot
NOINLINE_NOT_STARTS_WITH += load_package
NOINLINE_NOT_STARTS_WITH += finish_output
NOINLINE_NOT_STARTS_WITH += run
NOINLINE_NOT_STARTS_WITH += load
NOINLINE_NOT_STARTS_WITH += give_back_interval
NOINLINE_NOT_STARTS_WITH += write

NET_NOINLINE = $(MAIN).noinline.pnet
PS_NOINLINE = $(MAIN).noinline.ps

###############################################################################

PNETC_LIB += $(TRUNK)/application/lib

###############################################################################

WE_EXEC_LOAD += $(BUILD)/fvm-pc/fvm-pc/fake/libfvm-pc.so

WE_EXEC_LIBPATHS += $(PATH_LIB)

LIB_PATH = $(BUILD)/application/kdm:$(BUILD)/application/example/exec

###############################################################################

CXXINCLUDEPATHS += $(TRUNK)/fvm-pc/trunk
CXXINCLUDEPATHS += $(TRUNK)/fhglog/trunk
CXXINCLUDEPATHS += $(TRUNK)/application/example/exec
CXXINCLUDEPATHS += $(BUILD)/fhglog

# determine_size, do_load, do_write
CXXINCLUDEPATHS += $(TRUNK)/application/kdm/src

CXXINCLUDEPATHS += $(UTIL)
CXXINCLUDEPATHS += $(SEISLIB)

###############################################################################

PNETC    = $(BUILD)/xml/xml/parse/pnetc
WE_EXEC  = LD_LIBRARY_PATH=$(LIB_PATH):${LD_LIBRARY_PATH}
WE_EXEC += FHGLOG_level=MIN
WE_EXEC += FVM_PC_SHMSZ=`echo "$(SHMEM_PER_NODE)" | bc`
WE_EXEC += FVM_PC_FVMSZ=`echo "$(GPI_MEM_PER_NODE)" | bc`
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
