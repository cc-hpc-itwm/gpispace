
CXX = g++

CXXFLAGS += -O3
CXXFLAGS += -Wall
CXXFLAGS += -Wextra

CXXINCLUDEPATHS += .
CXXINCLUDEPATHS += $(BOOST_ROOT)/include

CXXLIBPATHS += $(BOOST_ROOT)/lib

LIBS += boost_program_options
LIBS += boost_thread
LIBS += boost_system

###############################################################################

.PHONY: default

default: gen.bin

###############################################################################

gen.o: gen.cpp writer.hpp queue.hpp buffer.hpp
	$(CXX) $(CXXFLAGS) $(addprefix -I,$(CXXINCLUDEPATHS)) -c $<

OBJS += gen.o

gen.bin: gen.o
	$(CXX) $< $(addprefix -L,$(CXXLIBPATHS)) $(addprefix -l,$(LIBS)) -o $@

BINS += gen.bin

###############################################################################

.PHONY: clean objclean

RM = rm

clean: objclean
	$(RM) -f $(BINS)

objclean:
	$(RM) -f $(OBJS)
