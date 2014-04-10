
ifndef MAIN
  MAIN = transform_file
endif

ifndef MAKECMDGOALS
 ifndef BUILDDIR
  	$(error please specify the build directory)
  endif
  ifndef INPUT_FILE
  	$(error input file not specified, please specify one in the command line)
  endif
  
  ifndef OUTPUT_FILE
  	$(error output file not specified, please specify one in the command line)
  endif
endif

ifeq ($(MAKECMDGOALS),run)
  ifndef BUILDDIR
  	$(error please specify the build directory)
  endif
endif

ifeq ($(MAKECMDGOALS),clean)
  ifndef BUILDDIR
  	$(error please specify the build directory)
  endif
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

# default values
CHUNK_SIZE=1024
READ_SLOTS=10
WRITE_SLOTS=20

PUT_PORT += input_file=\"$(INPUT_FILE)\"
PUT_PORT += output_file=\"$(OUTPUT_FILE)\"
PUT_PORT += chunk_size=$(CHUNK_SIZE)L
PUT_PORT += input_n_read_slots=$(READ_SLOTS)L
PUT_PORT += input_n_write_slots=$(WRITE_SLOTS)L

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)
CXXLIBRARYPATHS += $(SDPA_HOME)/lib

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
