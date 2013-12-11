
ifndef MAIN
  MAIN = hello_many
endif

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)/include
CXXLIBRARYPATHS += $(CURDIR)
PNETC_LINK_PREFIX += "libdir=$(CURDIR)"

###############################################################################

PUT_PORT += in=[]
PUT_PORT += in=[]

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
