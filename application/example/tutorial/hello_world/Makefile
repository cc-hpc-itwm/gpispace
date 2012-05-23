
ifndef MAIN
  MAIN = hello_many
endif

CXXINCLUDEPATHS += $(CURDIR)/include
CXXLIBRARAYPATHS += $(CURDIR)/lib

DEP += $(wildcard $(CURDIR)/*.xpnet)

###############################################################################

include ../common.make

###############################################################################

$(PUT): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUT) -p in=[] -p in=[] -p in=[]
