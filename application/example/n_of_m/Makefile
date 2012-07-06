
ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

###############################################################################

MAIN = n_of_m

###############################################################################

NOT_STARTS_WITH += scatter
NOT_STARTS_WITH += generate
NOT_STARTS_WITH += wait
NOT_STARTS_WITH += eat

###############################################################################

CXXINCLUDEPATHS += $(SDPA_INCLUDE)/process

CXXLIBRARYPATHS += $(SDPA_HOME)/libexec/sdpa

###############################################################################

ifndef M
  M = 25
endif

ifndef PARALLEL
  PARALLEL = 5
endif

ifndef CMD
  CMD = demo.bin
endif

PUT_PORT += m=$(M)
PUT_PORT += parallel=$(PARALLEL)
PUT_PORT += cmd=\"$(CMD)\"

###############################################################################

include $(HOME)/sdpa/application/common.mk
