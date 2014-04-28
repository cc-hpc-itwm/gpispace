
ifndef DIR_BUILD
  DIR_BUILD=.
endif

ifndef BOOST_ROOT
  ifndef SDPA_HOME
    $(error Neither BOOST_ROOT nor SDPA_HOME are set)
  else
    BOOST_ROOT := $(SDPA_HOME)/external/boost
  endif
else
  ifdef SDPA_HOME
    ifneq "$(BOOST_ROOT)" "$(SDPA_HOME)/external/boost"
      $(warning BOOST_ROOT is set and different from GSPC bundled version)
      $(warning BOOST_ROOT = $(BOOST_ROOT))
      $(warning GSPC_BUNDLED = $(SDPA_HOME)/external/boost)
    endif
  endif
endif

CXXFLAGS += -O3
CXXFLAGS += -Wall
CXXFLAGS += -Wextra

CXXINCLUDEPATHS += .
CXXINCLUDEPATHS += $(BOOST_ROOT)/include

CXXLIBPATHS += $(BOOST_ROOT)/lib

LIBS += boost_program_options
LIBS += boost_thread
LIBS += boost_system
LIBS += pthread

OBJ = $(DIR_BUILD)/gen.o
BIN = $(DIR_BUILD)/gen.bin

###############################################################################

.PHONY: default

default: $(BIN)

###############################################################################

$(OBJ): gen.cpp writer.hpp queue.hpp buffer.hpp
	$(CXX) $(CXXFLAGS) $(addprefix -I,$(CXXINCLUDEPATHS)) -c $< -o $@

$(BIN): $(OBJ)
	$(CXX) $< $(addprefix -L,$(CXXLIBPATHS)) $(addprefix -l,$(LIBS)) -o $@

###############################################################################

.PHONY: clean objclean

RM = rm

clean: objclean
	$(RM) -f $(BIN)

objclean:
	$(RM) -f $(OBJ)
