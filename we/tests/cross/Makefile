
ifndef MAIN
  MAIN = cross
endif

###############################################################################

PUT_PORT += n=10

NOT_STARTS_WITH += generate
NOT_ENDS_WITH += scatter

DEP += Makefile

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
