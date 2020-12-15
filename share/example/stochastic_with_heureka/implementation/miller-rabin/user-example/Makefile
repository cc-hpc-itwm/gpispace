# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

# requires: GMP_PREFIX = installation of GNU Multiple Precision Arithmatic Library
# requires: GSPC_PREFIX = installation of gpispace
# requires: SWH_PREFIX = installation of stochastic-with-heureka

includes+=-I $(SWH_PREFIX)/include
includes+=-I $(SWH_PREFIX)/example
includes+=-I $(GMP_PREFIX)/include
includes+=-I $(GSPC_PREFIX)/include
includes+=-I $(GSPC_PREFIX)/external/boost/include

library_dirs+=-L $(GSPC_PREFIX)/lib
library_dirs+=-L $(GMP_PREFIX)/lib
library_dirs+=-Wl,-rpath-link $(GSPC_PREFIX)/external/boost/lib

LDFLAGS+=-Wl,--no-undefined
LDFLAGS+=-Wl,--copy-dt-needed-entries

.PHONY: default
default: all

%.o: %.cpp
	$(CXX) --std=c++11 -fPIC $(includes) -o $@ -c $^

miller-rabin.so: miller-rabin.o util.o
	$(CXX) --std=c++11 -o $@ -shared $(library_dirs) $(LDFLAGS) -lwe-dev -lgmp -lgmpxx $^

generate-user-data: generate_user_data.cpp util.o
	$(CXX) --std=c++11 -o $@ $(includes) $(library_dirs) $(LDFLAGS) -lwe-dev -lgmp -lgmpxx $^

show-result: show_result.cpp
	$(CXX) --std=c++11 -o $@ $(includes) $(library_dirs) $(LDFLAGS) -lwe-dev $^

.PHONY: clean all
clean:
	$(RM) miller-rabin.so miller-rabin.o util.o
	$(RM) generate-user-data generate_user_data.o
	$(RM) show-result

all: generate-user-data miller-rabin.so show-result
