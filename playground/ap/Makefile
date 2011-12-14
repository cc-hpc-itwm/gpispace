DEFINES = -DUSE_STL_TR1=1
INCLUDES =-I/p/herc/itwm/hpc/soft/fhglog/1.2.0/intel/include
INCLUDES+=-I/p/herc/itwm/hpc/soft/sdpa/fvm-pc/include
INCLUDES+=-I/p/herc/itwm/hpc/ap/SDPA/sdpa/trunk
INCLUDES+=-I/p/herc/itwm/hpc/soft/boost/1.38/intel/include/boost-1_38
INCLUDES+=-I.

LINKDIRS=

#CXX = /opt/cluster/Intel/cce/9.1.047/bin/icpc
CXXFLAGS = -g -ggdb -wd383 -wd981 -O0 -fPIC $(DEFINES) $(INCLUDES)
LDFLAGS  = -shared $(LINKDIRS)

MOD_SRCS=$(wildcard *-mod.cpp)
MODS=$(patsubst %.cpp,lib%.so,$(MOD_SRCS))

default: all
all: $(MODS) Makefile

# make rules
%.o : %.cpp
	@echo "[CC] $@ (<= $^)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

lib%.so : %.o
	@echo "[MOD] $@ (<= $^)"
	@$(CXX) $(LDFLAGS) $^ -o $@

# special dependencies
libremig-mod.so: remig/fft.o remig/propagatorsOpt.o remig/utilsOpt.o
cfg.o: cfg.hpp
libcfg-mod.so: cfg.o

clean:
	rm -f *.o *.so

.PHONY: clean all default
