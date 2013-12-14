
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

$(BUILDDIR)/concurrent.o: concurrent.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(SDPA_HOME)/include -c $< -o $@

$(BUILDDIR)/libconcurrent.so: $(BUILDDIR)/concurrent.o
	$(CXX) -fPIC -shared -o $@ $< $(LDFLAGS)

.PHONY: clean-concurrent

clean-concurrent:
	$(RM) $(BUILDDIR)/concurrent.o
	$(RM) $(BUILDDIR)/libconcurrent.so
