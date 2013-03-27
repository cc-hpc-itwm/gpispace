
MAIN = avg_stddev

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)/src
CXXINCLUDEPATHS += $(BOOST_ROOT)/include

#CXXLIBRARYPATHS += $(CURDIR)/lib

###############################################################################

ifndef DATFILE
  DATFILE=$(CURDIR)/dat
endif

ifndef SIZE_FILE
  SIZE_FILE=838860800
endif

ifndef SIZE_CHUNK
  SIZE_CHUNK=8388608
endif

ifndef SIZE_BUFFER
  SIZE_BUFFER=8388608
endif

ifndef NUM_BUFFER
  NUM_BUFFER=10
endif

PUT_PORT += name_file=\"$(DATFILE)\"
PUT_PORT += size_file=$(SIZE_FILE)
PUT_PORT += size_chunk=$(SIZE_CHUNK)
PUT_PORT += size_buffer=$(SIZE_BUFFER)
PUT_PORT += num_buffer=$(NUM_BUFFER)

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
