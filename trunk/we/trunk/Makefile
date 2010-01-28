
DOPTS += $(CPP_MACROS)

WARN += all
WARN += error
# BOOST
##WARN += extra
WARN += init-self
WARN += missing-include-dirs
WARN += switch-default
# BOOST!
##WARN += switch-enum
WARN += no-pragmas
WARN += float-equal
# BOOST!
##WARN += shadow 
WARN += enum-compare
WARN += missing-declarations
# BOOST!
##WARN += missing-noreturn
WARN += redundant-decls

EOPTS += pedantic Wno-long-long

CXX_FLAGS = $(addprefix -W,$(WARN))
#CXX_FLAGS += -g
CXX_FLAGS += -O3
CXX_FLAGS += $(addprefix -,$(EOPTS))
CXX_FLAGS += $(addprefix -D,$(DOPTS))

CXX_INCLUDE = .
CXX_INCLUDE += ${BOOST_ROOT}/include

CXX_LIBS += pthread
CXX_LIBS += boost_thread

HPP = $(wildcard *.hpp)
CPP = $(wildcard *.cpp)

.PHONY: all

all: $(addsuffix .bin,$(basename $(CPP)))

%.bin: %.cpp $(HPP) Makefile
	$(CXX) $(CXX_FLAGS) $(addprefix -l, $(CXX_LIBS)) \
                            $(addprefix -I, $(CXX_INCLUDE)) $< -o $@

%.bin.gprof: %.bin
	./$^
	gprof ./$^ > $@

%.bin.gprof.dot: %.bin.gprof
	python gprof2dot.py <$< >$@ 2>$*.bin.gprof.map

%.bin.gprof.ps: %.bin.gprof.dot
	dot -Tps $< >$@

.PHONY: clean

clean:
	rm -rf *.bin
	rm -rf *.dot
	rm -rf *.map
	rm -rf *.ps
	rm -rf *.out
	rm -rf *.gprof
	rm -rf *.map