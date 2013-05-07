# Start SDPA
#
#     #proc   GPI-mem    SHM-mem    init     finalize      
# sdpa boot -m 32*2**30 -s 2*2**30 INI:1x1,0 FIN:1x1,0 LOAD:2,1024*2**20 MAP:1 MAP+RED:10 RED:1
#

ifndef MAIN
  MAIN = atomic
endif

###############################################################################

NOT_STARTS_WITH += dup
NOT_STARTS_WITH += triple
NOT_STARTS_WITH += generate
NOT_STARTS_WITH += release
NOT_STARTS_WITH += check
NOT_STARTS_WITH += gen

###############################################################################

WE_EXEC_LOAD += $(SDPA_HOME)/libexec/sdpa/fake/libfvm-pc.so

###############################################################################

PUT_PORT += n_A=100
PUT_PORT += n_B=100
PUT_PORT += shared_file=\"/var/tmp/atomic.txt\"
-include example.mk

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)/..
CXXLIBRARYPATHS += $(SDPA_HOME)/lib

###############################################################################
WE_EXEC_WORKER = 1
ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
