
CXX_FLAGS = -Wall -Werror -O3
#CXX = g++
CXX_INCLUDE = .
CXX_INCLUDE += ${BOOST_ROOT}/include

.PHONY: all

all: t.bin b.bin

%.bin: %.cpp net.hpp
	$(CXX) $(CXX_FLAGS) $(addprefix -I,$(CXX_INCLUDE)) $< -o $@

%.bin.gprof: %.bin
	./$^
	gprof ./$^ > $@

%.bin.gprof.dot: %.bin.gprof
	python gprof2dot.py <$< >$@ 2>$*.bin.gprof.fmap

%.bin.gprof.ps: %.bin.gprof.dot
	perl untempl.pl <$*.bin.gprof.fmap >$*.bin.gprof.map
	dot -Tps $< >$@

.PHONY: clean

clean:
	rm -rf *.bin
	rm -rf *.dot
	rm -rf *.map
	rm -rf *.ps
	rm -rf *.out
	rm -rf *.gprof
	rm -rf *.fmap
	rm -rf *.map