
MAIN = map

-include Makefile.builddir

###############################################################################

include Makefile.put

DEP += Makefile Makefile.put

###############################################################################

CXXINCLUDEPATHS += $(CURDIR)
CXXLIBRARYPATHS += $(BUILDDIR)
CXXFLAGS += -fPIC

###############################################################################

MAP_IMPL = $(BUILDDIR)/libmap-impl.so
DEP += $(MAP_IMPL)

###############################################################################

WE_EXEC_ENV += FVM_PC_SHMSZ=524288000
WE_EXEC_ENV += FVM_PC_FVMSZ=524288000

WE_EXEC_LOAD += $(SDPA_HOME)/libexec/sdpa/fake/libfvm-pc.so
WE_EXEC_CMD = we-exec
WE_EXEC_WORKER = 1

###############################################################################

ifndef SDPA_HOME
 $(error variable SDPA_HOME not set)
endif

CLEAN += clean_log_impl
CLEAN += clean_map_impl

include $(SDPA_HOME)/share/sdpa/make/common.mk

###############################################################################

$(BUILDDIR)/log_impl.o: log_impl.cpp
	$(CXX) $(CXXFLAGS) -isystem $(SDPA_HOME)/include -isystem $(SDPA_HOME)/external/boost/include -I$(CURDIR) -c $< -o $@

.PHONY: clean_log_impl

clean_log_impl:
	$(RM) $(BUILDDIR)/log_impl.o

###############################################################################

$(MAP_IMPL): $(BUILDDIR)/log_impl.o
	$(CXX) -shared $^ -o $@ $(LDFLAGS) -L$(SDPA_HOME)/external/boost/lib -lboost_system

.PHONY: clean_map_impl

clean_map_impl:
	$(RM) $(MAP_IMPL)
