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

ifndef DIR_BUILD
  $(error DIR_BUILD not set)
endif

LIB_PATH = $(DIR_BUILD)
INC_PATH = $(CURDIR)/../include

OBJ = $(LIB_PATH)/sum.o

CFLAGS += -I$(INC_PATH)
CFLAGS += -fPIC

###############################################################################

.PHONY: default all

default: all
all: $(OBJ)

###############################################################################

$(LIB_PATH):
	@mkdir -p "$@"

$(OBJ): sum.c $(LIB_PATH)
	$(CC) $(CFLAGS) -c $< -o $@
