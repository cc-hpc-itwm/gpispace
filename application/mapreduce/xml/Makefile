
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
PUT_PORT += partition_slot_size=20000
PUT_PORT += reduce_slot_size=200000
PUT_PORT += n_part=5
PUT_PORT += input_n_read_slots=20
PUT_PORT += input_n_map_slots=10

# test with reduce_tpl.xpnet
#PUT_PORT += -p config=\"empty\" -p reduce_info_in=[avail_slots:=10,slots_to_reduce:=\"1\ 5\ 7\ 8\ 3\",part_used:=\"100\ 200\ 300\ 400\ 500\ 600\ 700\ 800\ 900\ 10000\",red_used:=\"0\ 0\ 0\ 0\ 0\ 0\ 0\ 0\ 0\ 0\"]

# test with reduce.xpnet
#PUT_PORT += -p reduce_info_in=[avail_slots:=10,slots_to_reduce:=\"1\ 5\ 7\ 8\ 3\",part_used:=\"100\ 200\ 300\ 400\ 500\ 600\ 700\ 800\ 900\ 10000\",red_used:=\"0\ 0\ 0\ 0\ 0\ 0\ 0\ 0\ 0\ 0\"]

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)/..
CXXLIBRARYPATHS += $(SDPA_HOME)/lib

###############################################################################
WE_EXEC_WORKER = 1
ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
