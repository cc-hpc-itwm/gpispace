
ifndef MAIN
  MAIN = gpi
endif

###############################################################################

PNETC_OPTS += -I..

###############################################################################

NOT_STARTS_WITH += generate
#NOT_ENDS_WITH += work
NOT_STARTS_WITH += wait

###############################################################################

PUT_PORT += n=12L
PUT_PORT += c=12L
PUT_PORT += config=Struct[description:=\"gpi-test\"]

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

WE_EXEC_LOAD += $(SDPA_HOME)/libexec/sdpa/fake/libfvm-pc.so

include $(SDPA_HOME)/share/sdpa/make/common.mk
