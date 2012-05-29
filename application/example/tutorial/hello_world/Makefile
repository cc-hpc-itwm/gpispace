
ifndef MAIN
  MAIN = hello_many
endif

CXXINCLUDEPATHS += $(CURDIR)/include
CXXLIBRARYPATHS += $(CURDIR)/lib

DEP += $(wildcard $(CURDIR)/*.xpnet)

###############################################################################

include ../common.mk

###############################################################################

$(PUT): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUT) -p in=[] -p in=[]

