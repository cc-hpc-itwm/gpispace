
ifndef MAIN
  MAIN = transform_file
endif

ifndef MAKECMDGOALS
 ifndef BUILDDIR
  	$(error please specify the build directory)
  endif
  ifndef INPUT_FILE
  	$(error INPUT_FILE not set, please specify the name of the input file)
  endif
  
  ifndef OUTPUT_FILE
  	$(error OUTPUT_FILE not set, please specify the name of the output file)
  endif
endif

ifeq ($(MAKECMDGOALS),run)
  ifndef BUILDDIR
	$(error BUILDDIR not set, please specify the build directory)
  endif
endif

ifeq ($(MAKECMDGOALS),clean)
  ifndef BUILDDIR
  	$(error  BUILDDIR not set, please specify the build directory)
  endif
endif

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
