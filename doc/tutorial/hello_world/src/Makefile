# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
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

ifndef BUILDDIR
  $(error BUILDDIR not set)
endif

LIB_PATH = $(BUILDDIR)
INC_PATH = $(CURDIR)/../include

OBJ_HELLO = $(LIB_PATH)/hello_world.o
OBJ_HELLO2 = $(LIB_PATH)/hello2.o
OBJ_LIBHELLO = $(LIB_PATH)/libhello_util.so

OBJ += $(OBJ_HELLO)
OBJ += $(OBJ_HELLO2)
OBJ += $(OBJ_LIBHELLO)

CXXFLAGS += -I$(INC_PATH)
CXXFLAGS += -fPIC

CFLAGS += -I$(INC_PATH)
CFLAGS += -fPIC

###############################################################################

.PHONY: default all

default: all
all: $(OBJ)

###############################################################################

$(LIB_PATH):
	@mkdir -p "$@"

$(OBJ_HELLO): hello_world.cpp $(LIB_PATH)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_HELLO2): hello2.c $(LIB_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_LIBHELLO): hello_util.c $(LIB_PATH)
	$(CC) $(CFLAGS) -shared $< -o $@
