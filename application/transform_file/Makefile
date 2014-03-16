
ifndef MAIN
  MAIN = transform_file
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

PUT_PORT += input_file=\"/scratch/rotaru/data/words\"
PUT_PORT += output_file=\"/scratch/rotaru/data/out_transf_words.txt\"
PUT_PORT += chunk_size=1024
PUT_PORT += input_n_read_slots=10
PUT_PORT += input_n_write_slots=20

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)/..
CXXLIBRARYPATHS += $(SDPA_HOME)/lib

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
