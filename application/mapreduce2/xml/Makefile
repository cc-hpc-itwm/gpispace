
ifndef MAIN
  MAIN = mapreduce
endif

###############################################################################

NOT_STARTS_WITH += dup
NOT_STARTS_WITH += triple
NOT_STARTS_WITH += generate
NOT_STARTS_WITH += release
NOT_STARTS_WITH += check

###############################################################################

WE_EXEC_LOAD += $(SDPA_HOME)/libexec/sdpa/fake/libfvm-pc.so

###############################################################################

PUT_PORT += input_file=\"$(PWD)/../example/loremipsum.txt\"
PUT_PORT += output_file=\"$(PWD)/out.txt\"
PUT_PORT += read_chunk_size=1024
PUT_PORT += map_chunk_size=2048
PUT_PORT += partition_slot_size=200000
PUT_PORT += reduce_slot_size=200000
PUT_PORT += n_part=5
PUT_PORT += input_n_read_slots=10
PUT_PORT += input_n_map_slots=10

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
