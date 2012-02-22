
MAIN = tracebytrace

SDPA_INCLUDE = $(SDPA_HOME)/include
SDPA_LIBEXEC = $(SDPA_HOME)/libexec
SDPA_XML_LIB = $(SDPA_HOME)/share/sdpa/xml/lib

LIB_DESTDIR = $(DESTDIR)/$(MAIN)/lib/

PATH_MAIN  = $(CURDIR)
UTIL       = $(PATH_MAIN)/util

TEE      = /usr/bin/tee
DOT      = /usr/bin/dot -Tps -Grankdir=LR
RM       = /bin/rm -rf
TOUCH    = touch
CAT      = /bin/cat
CP       = /bin/cp
MKDIR    = mkdir

.PHONY: default
default: all

###############################################################################

XML_MAIN    = $(PATH_MAIN)/xml/$(MAIN).xml
XML_MAIN_IN = $(XML_MAIN).in
MK_XML      = $(PATH_MAIN)/mk.xml

###############################################################################

DEP += xml/generate/package.xml
DEP += xml/generate/slot.xml
DEP += util/print.hpp
DEP += Makefile
DEP += $(UTIL)/print.hpp
DEP += $(UTIL)/comm.hpp

NET = $(MAIN).pnet
GEN = gen
OUT = $(MAIN).out
PS  = $(MAIN).ps

PATH_LIB = $(PATH_MAIN)/$(GEN)/pnetc/op

LIB = $(PATH_LIB)/lib$(MAIN).so

###############################################################################

FILE_INPUT       = /u/herc/rahn/test.dat
TYPE_INPUT       = su
FILE_OUTPUT      = /u/herc/rahn/test.dat.out
TYPE_OUTPUT      = su
CMD              = cat
SKIPPED_TRACES   = 0
MAX_TRACE_COUNT  = -1
SHMEM_PER_NODE   = 4240 * 2
GPI_MEM_PER_NODE = 4240 * 6

$(XML_MAIN): $(XML_MAIN_IN) $(DEP)
	$(CAT) $< | $(MK_XML) "$(FILE_INPUT)" "$(TYPE_INPUT)" "$(FILE_OUTPUT)" "$(TYPE_OUTPUT)" "$(CMD)" "$(SKIPPED_TRACES)" "$(MAX_TRACE_COUNT)" "$(SHMEM_PER_NODE)" "$(GPI_MEM_PER_NODE)" "$(SDPA_LIBEXEC)" > $@

###############################################################################

NOINLINE_NOT_ENDS_WITH += generate
NOINLINE_NOT_ENDS_WITH += generate_slot
NOINLINE_NOT_ENDS_WITH += generate_package

NOINLINE_NOT_STARTS_WITH += dup
NOINLINE_NOT_STARTS_WITH += scatter
NOINLINE_NOT_STARTS_WITH += eat
NOINLINE_NOT_STARTS_WITH += wait

NET_NOINLINE = $(MAIN).noinline.pnet
PS_NOINLINE = $(MAIN).noinline.ps

###############################################################################

WE_EXEC_LOAD += $(SDPA_LIBEXEC)/fake/libfvm-pc.so

WE_EXEC_LIBPATHS += $(PATH_LIB)

###############################################################################

CXXINCLUDEPATHS += $(SDPA_INCLUDE)
CXXINCLUDEPATHS += $(SDPA_INCLUDE)/process

# determine_size, do_load, do_write
CXXINCLUDEPATHS += $(SDPA_INCLUDE)/seis

# print.hpp, comm.hpp
CXXINCLUDEPATHS += $(UTIL)

###############################################################################

PNETC    = $(SDPA_HOME)/bin/pnetc
WE_EXEC  = LD_LIBRARY_PATH=$(SDPA_LIBEXEC):${LD_LIBRARY_PATH}
WE_EXEC += $(SDPA_HOME)/bin/we-exec --w 1
PNET2DOT = $(SDPA_HOME)/bin/pnet2dot

###############################################################################

PNETC += $(addprefix -I ,$(SDPA_XML_LIB))
PNETC += --gen-ldflags=-L$(SDPA_HOME)/libexec
PNETC += --gen-ldflags=-L$(SDPA_HOME)/lib
PNETC += --gen-cxxflags=-O3
PNETC += --gen-cxxflags=-I$(SDPA_HOME)/include
PNETC += --gen-cxxflags=-I../../../kdm/src
PNETC += --gen-cxxflags=-I../util
PNETC += --gen-cxxflags=-Wno-deprecated-declarations
PNETC += --force-overwrite-file=true
PNETC += --Woverwrite-file=false
PNETC += --Wbackup-file=false

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

.PHONY: ps all out net

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

$(LIB_DESTDIR):
	$(MKDIR) -p "$(LIB_DESTDIR)"

.PHONY: install

install: all $(GEN) $(LIB_DESTDIR)
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
	$(RM) $(XML_MAIN)
