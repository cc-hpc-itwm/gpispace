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

extended_add_library (NAME determine_size
  TYPE SHARED
  SOURCES "determine_size.cpp"
          "swap_bytes.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/seis"
  INSTALL
  INSTALL_DESTINATION "libexec/gspc"
)
extended_add_library (NAME do_load
  TYPE SHARED
  SOURCES "do_load.cpp"
          "swap_bytes.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/seis"
  INSTALL
  INSTALL_DESTINATION "libexec/gspc"
)
extended_add_library (NAME do_write
  TYPE SHARED
  SOURCES "do_write.cpp"
          "swap_bytes.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/seis"
  INSTALL
  INSTALL_DESTINATION "libexec/gspc"
)

install (FILES do_load.hpp determine_size.hpp do_write.hpp
  DESTINATION include/seis
)
