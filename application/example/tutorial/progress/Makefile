
ifndef MAIN
  MAIN = progress
endif

###############################################################################

PNETC_OPTS += -I..
CXXINCLUDEPATHS += $(SDPA_HOME)/include
CXXLIBRARYPATHS += $(CURDIR)/lib

###############################################################################

NOT_STARTS_WITH += generate
NOT_ENDS_WITH += work
NOT_STARTS_WITH += wait

###############################################################################

PUT_PORT += ticks=100L
PUT_PORT += $(PUT_EXTRA)

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
