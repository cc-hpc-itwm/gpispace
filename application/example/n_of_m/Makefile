
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

PUT_PORT += m=25L
PUT_PORT += parallel=2L
PUT_PORT += cmd=\"demo.bin\"

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
