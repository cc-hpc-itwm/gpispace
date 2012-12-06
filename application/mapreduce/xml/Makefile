# Start SDPA
#
#           #proc   GPI-mem    SHM-mem    init     finalize        read_chunk_size  def-shm 4xdef 2xdef-shm 4xdef-shm
# sdpa boot -p 4 -m 5*2**30 -s 64*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,8*2**20          WRITE:2 MAP RED:2   PAR
#

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

PUT_PORT += input_file=\"/fhgfs/HPC/rotaru/data/american.300\"
PUT_PORT += output_file=\"/fhgfs/HPC/rotaru/data/out.txt\"
PUT_PORT += read_chunk_size=8194304
#PUT_PORT += map_chunk_size=4194304
PUT_PORT += map_chunk_size=12582912
PUT_PORT += partition_slot_size=33554432
#PUT_PORT += partition_slot_size=16777216
#PUT_PORT += reduce_slot_size=33554432
PUT_PORT += reduce_slot_size=67108864
PUT_PORT += n_part=20
PUT_PORT += input_n_read_slots=50
PUT_PORT += input_n_map_slots=40
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
