# requires: GMP_PREFIX = installation of GNU Multiple Precision Arithmatic Library
# requires: GSPC_PREFIX = installation of gpispace
# requires: SWH_PREFIX = installation of stochastic-with-heureka

CXXFLAGS+=-I $(SWH_PREFIX)/include
CXXFLAGS+=-I $(SWH_PREFIX)/example
CXXFLAGS+=-I $(GMP_PREFIX)/include
CXXFLAGS+=-I $(GSPC_PREFIX)/include
CXXFLAGS+=-I $(GSPC_PREFIX)/external/boost/include
CXXFLAGS+=--std=c++11
CXXFLAGS+=-fPIC

LDFLAGS+=-L $(GSPC_PREFIX)/lib
LDFLAGS+=-L $(GMP_PREFIX)/lib
LDFLAGS+=-Wl,-rpath-link $(GSPC_PREFIX)/external/boost/lib
LDFLAGS+=-Wl,-rpath $(GSPC_PREFIX)/lib
LDFLAGS+=-Wl,-rpath $(GMP_PREFIX)/lib
LDFLAGS+=-Wl,--no-undefined
LDFLAGS+=-lwe-dev
LDFLAGS+=-lgmp
LDFLAGS+=-lgmpxx

.PHONY: default
default: all

miller-rabin.so: miller-rabin.o util.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ -shared $^

generate-user-data: generate_user_data.cpp util.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

show-result: show_result.cpp util.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: clean all
clean:
	$(RM) miller-rabin.so miller-rabin.o util.o
	$(RM) generate-user-data generate_user_data.o
	$(RM) show-result

all: generate-user-data miller-rabin.so show-result
