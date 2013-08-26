
ifndef MAIN
  MAIN = work_and_wait
endif

###############################################################################

PNETC_OPTS += -I..

###############################################################################

NOT_STARTS_WITH += generate
NOT_ENDS_WITH += work
NOT_STARTS_WITH += wait

###############################################################################

PUT_PORT += n=12L
PUT_PORT += $(PUT_EXTRA)

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
