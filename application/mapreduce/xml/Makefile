# Start SDPA
#
#           #proc   GPI-mem    SHM-mem    init     finalize        read_chunk_size  def-shm 4xdef 2xdef-shm 4xdef-shm
# sdpa boot -p 4 -m 5*2**30 -s 64*2**20 INI:1x1,0 FIN:1x1,0 LOAD:2,8*2**20   MAP:2 RED:8  
#

ifndef MAIN
  MAIN = mapreduce_ml
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

#PUT_PORT += input_file=\"/scratch/rotaru/data/words.500\"
PUT_PORT += input_file_1=\"/scratch/rotaru/data/words.1000\"
PUT_PORT += input_file_2=\"/scratch/rotaru/data/words.1000\"
PUT_PORT += output_file=\"/scratch/rotaru/data/out.txt\"
PUT_PORT += read_chunk_size=130108864 #260108864
PUT_PORT += map_chunk_size=190943004 #360943004
#PUT_PORT += map_chunk_size=35582912
PUT_PORT += partition_slot_size=2097152 #63554432
#PUT_PORT += partition_slot_size=2097152
#PUT_PORT += reduce_slot_size=61777216
PUT_PORT += reduce_slot_size=150008864
PUT_PORT += n_part=28
PUT_PORT += input_n_read_slots=100
PUT_PORT += input_n_map_slots=100
PUT_PORT += input_n_slots_per_part=6
PUT_PORT += input_n_slots_per_red=6
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
