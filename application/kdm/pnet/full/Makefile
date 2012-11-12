
ifndef MAIN
  MAIN = kdm
endif

###############################################################################

NOT_STARTS_WITH += dup
NOT_STARTS_WITH += triple
NOT_STARTS_WITH += generate
NOT_STARTS_WITH += release
NOT_STARTS_WITH += check

###############################################################################

WE_EXEC_LOAD += $(SDPA_HOME)/libexec/sdpa/fake/libfvm-pc.so

###############################################################################

#PUT_PORT += input_n_map_slots=10

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)/..
CXXLIBRARYPATHS += $(SDPA_HOME)/lib

###############################################################################
WE_EXEC_WORKER = 1
ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
