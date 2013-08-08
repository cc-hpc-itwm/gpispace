MAIN = stresstest

###############################################################################

NOT_STARTS_WITH += generate
NOT_STARTS_WITH += scatter
NOT_STARTS_WITH += inner
NOT_STARTS_WITH += wait

###############################################################################

CXXLIBRARYPATHS += $(SDPA_HOME)/lib

WE_EXEC_ENV += FVM_PC_SHMSZ=524288000
WE_EXEC_ENV += FVM_PC_FVMSZ=524288000

WE_EXEC_LOAD += $(SDPA_HOME)/libexec/sdpa/fake/libfvm-pc.so
WE_EXEC_CMD = we-exec

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk

WE_EXEC_LIBPATHS = $(SDPA_HOME)/libexec/sdpa
