
ifndef MAIN
  MAIN = gen_grid_points
endif

###############################################################################

PNETC_OPTS += -I$(SDPA_HOME)/share/sdpa/xml

PUT_PORT += grid='Struct[ll:=[x:=0.0,y:=0.0],ur:=[x:=1.0,y:=3.0],s:=[x:=1.0,y:=1.5]]'

NOT_STARTS_WITH += dup
NOT_STARTS_WITH += rep
NOT_STARTS_WITH += gene

###############################################################################

DEP += Makefile

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
