
CXX_FLAGS = -Wall -Werror -g
#CXX = g++
CXX_INCLUDE = .
CXX_INCLUDE += ${BOOST_ROOT}/include

.PHONY: all

all: t.bin b.bin

%.bin: %.cpp net.hpp
	$(CXX) $(CXX_FLAGS) $(addprefix -I,$(CXX_INCLUDE)) $< -o $@

.PHONY: clean

clean:
	rm -rf *.bin