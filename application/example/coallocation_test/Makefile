
ifndef MAIN
  MAIN = coallocation_test
#  MAIN = mapreduce
endif

###############################################################################

NOT_STARTS_WITH += dup
NOT_STARTS_WITH += generate
NOT_STARTS_WITH += gen

###############################################################################

WE_EXEC_LOAD += $(SDPA_HOME)/libexec/sdpa/fake/libfvm-pc.so

###############################################################################

#PUT_PORT += input_file=\"/scratch/rotaru/data/words.110000\"
PUT_PORT += n_tasks_A=10L
PUT_PORT += n_workers_A=2L
PUT_PORT += n_tasks_B=10L
PUT_PORT += n_workers_B=6L
-include example.mk

###############################################################################
CXXINCLUDEPATHS += $(CURDIR)/..
CXXLIBRARYPATHS += $(SDPA_HOME)/lib
CXXLIBRARYPATHS += ${BOOST_ROOT}/lib
###############################################################################
WE_EXEC_WORKER = 1
ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
