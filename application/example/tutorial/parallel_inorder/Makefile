
ifndef MAIN
  MAIN = n_of_m
endif

DEP += $(wildcard *.xpnet)
DEP += ../credit/sequence_forward_credit.xpnet
DEP += ../template/three.xpnet

PNETC_OPTS += -I..

NOT_STARTS_WITH += generate
#NOT_ENDS_WITH += work
NOT_STARTS_WITH += wait

WE_EXEC_WORKER = 5

###############################################################################

include ../common.mk

###############################################################################

$(PUT): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUT) -p n=$(WE_EXEC_WORKER) \
                                           -p c=$(WE_EXEC_WORKER) \
                                           -p config=[description:=\"test\"]
