
ifndef MAIN
  MAIN = sum
endif

CXXINCLUDEPATHS += $(CURDIR)/include

###############################################################################

WE_EXEC_CMD = we-eval

include ../common.mk

###############################################################################

$(PUT): $(NET)
	$(PNETPUT) --if $(NET) --of $(PUT) -p p.x=3 -p p.y=4
