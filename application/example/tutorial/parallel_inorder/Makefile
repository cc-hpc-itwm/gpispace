
ifndef MAIN
  MAIN = n_of_m
endif

###############################################################################

PNETC_OPTS += -I..

###############################################################################

NOT_STARTS_WITH += generate
#NOT_ENDS_WITH += work
NOT_STARTS_WITH += wait

###############################################################################

WE_EXEC_WORKER = 5

PUT_PORT += n=$(WE_EXEC_WORKER)L
PUT_PORT += c=$(WE_EXEC_WORKER)L
PUT_PORT += config=Struct[description:=\"test\"]

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
