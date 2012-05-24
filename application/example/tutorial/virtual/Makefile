
ifndef MAIN
  MAIN = work_and_wait
endif

DEP = $(wildcard *.xpnet)
DEP = ../sequence/sequence.xpnet
DEP = ../template/dup.xpnet

PNETC_OPTS += -I..

NOT_STARTS_WITH += generate
#NOT_ENDS_WITH += work
NOT_STARTS_WITH += wait

###############################################################################

include ../common.mk

###############################################################################

$(PUT): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUT) -p n=5
