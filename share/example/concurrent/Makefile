
MAIN = concurrent

###############################################################################

NOT_STARTS_WITH += generate
NOT_STARTS_WITH += dup
NOT_STARTS_WITH += wait

###############################################################################

PUT_PORT = N=1000L

DEP += $(BUILDDIR)/libconcurrent.so
CLEAN += clean-concurrent

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

include $(SDPA_HOME)/share/sdpa/make/common.mk

WE_EXEC_LIBPATHS := $(BUILDDIR)

ifndef BOOST_ROOT
  ifndef SDPA_HOME
    $(error Neither BOOST_ROOT nor SDPA_HOME are set)
  else
    BOOST_ROOT := $(SDPA_HOME)/external/boost
  endif
else
  ifneq "$(BOOST_ROOT)" "$(SDPA_HOME)/external/boost"
    $(warning "BOOST_ROOT is set and different from GSPC bundled version")
    $(warning "BOOST_ROOT = $(BOOST_ROOT)")
    $(warning "GSPC_BUNDLED = $(SDPA_HOME)/external/boost")
  endif
endif

$(BUILDDIR)/concurrent.o: concurrent.cpp
	$(CXX) -fPIC $(CXXFLAGS) -isystem $(SDPA_HOME)/include -isystem $(BOOST_ROOT)/include -c $< -o $@

$(BUILDDIR)/libconcurrent.so: $(BUILDDIR)/concurrent.o
	$(CXX) -fPIC -shared -o $@ $< $(LDFLAGS)

.PHONY: clean-concurrent

clean-concurrent:
	$(RM) $(BUILDDIR)/concurrent.o
	$(RM) $(BUILDDIR)/libconcurrent.so
