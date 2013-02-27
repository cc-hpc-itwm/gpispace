
MAIN = avg_stddev

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)/src
CXXINCLUDEPATHS += $(BOOST_ROOT)/include

#CXXLIBRARYPATHS += $(CURDIR)/lib

###############################################################################

PUT_PORT += name_file=\"$(CURDIR)/dat\"
PUT_PORT += size_file=838860800
PUT_PORT += size_chunk=8388608
PUT_PORT += size_buffer=8388608
PUT_PORT += num_buffer=10

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk
