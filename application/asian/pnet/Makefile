
ifndef MAIN
  MAIN = wrapper
endif

XML = $(CURDIR)/xml/$(MAIN).xml

###############################################################################

ifndef SDPA_HOME
  $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk

###############################################################################

XML_MAIN = xml/wrapper.xml
MK_XML = $(CURDIR)/mk.xml

DEP += Makefile
DEP += xml/wrapper.xml
DEP += xml/collect.xml
DEP += xml/asian.xml
DEP += xml/run.xml
DEP += run_impl.hpp

NOT_ENDS_WITH += gen

NOT_STARTS_WITH += asian_id
NOT_STARTS_WITH += asian_eps1
NOT_STARTS_WITH += asian_eps2
NOT_STARTS_WITH += asian_delta
NOT_STARTS_WITH += collect

CXXINCLUDEPATHS += $(SDPA_INCLUDE)/process
CXXINCLUDEPATHS += $(CURDIR)
CXXINCLUDEPATHS += $(SDPA_HOME)/include
CXXINCLUDEPATHS += ..

CXXLIBRARYPATHS += $(SDPA_HOME)/libexec
CXXLIBRARYPATHS += $(SDPA_HOME)/lib
CXXLIBRARYPATHS += $(SDPA_HOME)/libexec/sdpa

WE_EXEC_LOAD += $(SDPA_HOME)/libexec/sdpa/libprocess.so

PUT_PORT += param=Struct[S:=0.1,K:=0.1,T:=0.1,sigma:=0.1,r:=0.1,r:=0.1,d:=0.1,FirstFixing:=1,FixingsProJahr:=3.0,AnzahlderDividende:=10,n:=10,rowID:=10,bin:=\"bin\",epsilon:=0.1,delta:=0.1,iterations_per_run:=10,nThreads:=1]
