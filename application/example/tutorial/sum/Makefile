
ifndef MAIN
  MAIN = sum_many
endif

CXXINCLUDEPATHS += $(CURDIR)/include

DEP += $(wildcard *.xpnet)

###############################################################################

include ../common.mk

###############################################################################

$(PUT): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUT) -p p=[x:=3,y:=4] -p p=[x:=-2,y:=3]
