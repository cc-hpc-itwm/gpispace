
MAIN = pi

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

XML_MAIN    = $(PATH_MAIN)/xml/$(MAIN).xml
XML_MAIN_IN = $(XML_MAIN).in
MK_XML      = $(PATH_MAIN)/mk.xml

###############################################################################

DEP += Makefile

NET = $(MAIN).pnet
GEN = gen
OUT = $(MAIN).out
PS  = $(MAIN).ps

PATH_LIB = $(PATH_MAIN)/$(GEN)/pnetc/op

LIB = $(PATH_LIB)/lib$(MAIN).so

###############################################################################

NUM_PACKET = 200
POINTS_PER_PACKET = 4 * 10^8
CREDIT_GENERATE = 64
CREDIT_RUN = 32
CREDIT_GET_KEY = 1
SEED = 271828

###############################################################################

#NOINLINE_NOT_ENDS_WITH += generate

#NOINLINE_NOT_STARTS_WITH += run
#NOINLINE_NOT_STARTS_WITH += get_key
#NOINLINE_NOT_STARTS_WITH += generate

NET_NOINLINE = $(MAIN).noinline.pnet
PS_NOINLINE = $(MAIN).noinline.ps

###############################################################################

WE_EXEC_LIBPATHS += $(PATH_LIB)

###############################################################################

CXXINCLUDEPATHS += $(SDPA_INCLUDE)

###############################################################################

PNETC    = $(SDPA_HOME)/bin/pnetc
WE_EXEC += LD_LIBRARY_PATH=$(SDPA_LIBEXEC):${LD_LIBRARY_PATH}
WE_EXEC += $(SDPA_HOME)/bin/we-exec
WE_EXEC +=  --w 1
PNET2DOT = $(SDPA_HOME)/bin/pnet2dot

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

default: all

ps: $(PS) $(PS_NOINLINE)

net: $(NET)

all: ps net $(GEN)

###############################################################################

$(XML_MAIN): $(XML_MAIN_IN) $(DEP)
	$(CAT) $< | $(MK_XML) "$(NUM_PACKET)" \
                              "$(POINTS_PER_PACKET)" \
                              "$(CREDIT_GENERATE)" \
                              "$(CREDIT_RUN)" \
                              "$(CREDIT_GET_KEY)" \
                              "$(SEED)" \
                              > $@

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
	$(RM) $(XML_MAIN)
