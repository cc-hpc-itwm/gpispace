
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

PUT_PORT += n=12L
PUT_PORT += c=4L
PUT_PORT += config=Struct[description:=\"test\"]

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
