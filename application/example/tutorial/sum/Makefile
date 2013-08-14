
ifndef MAIN
  MAIN = sum_many
endif

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)/include
PNETC_LINK_PREFIX += "libdir=$(CURDIR)"

###############################################################################

PUT_PORT += p=[x:=3L,y:=4L]
PUT_PORT += p=[x:=-2L,y:=3L]

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
