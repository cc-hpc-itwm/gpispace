# Start SDPA
#
#     #proc   GPI-mem    SHM-mem    init     finalize      
# sdpa boot -m 32*2**30 -s 2*2**30 INI:1x1,0 FIN:1x1,0 LOAD:2,1024*2**20 MAP:1 MAP+RED:10 RED:1
#

ifndef MAIN
  MAIN = mapreduce_ml
#  MAIN = mapreduce
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

#PUT_PORT += input_file=\"/scratch/rotaru/data/words.20000\"
PUT_PORT += input_file_1=\"/scratch/rotaru/data/words.1000\"
PUT_PORT += input_file_2=\"/scratch/rotaru/data/words.1000\"
PUT_PORT += output_file=\"/scratch/rotaru/data/out_2x1000.txt\"
PUT_PORT += read_chunk_size=130108864 #130108864 #260108864
PUT_PORT += map_chunk_size=180943004 #190943004 #190943004 #360943004
#PUT_PORT += map_chunk_size=360943004 #35582912
PUT_PORT += partition_slot_size=2097152 #63554432 #2097152
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
