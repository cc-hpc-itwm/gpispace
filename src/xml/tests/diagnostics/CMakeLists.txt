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

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/function_description_with_unknown_port.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/function_description_with_unknown_port.cpp
  @ONLY
)

fhg_add_test (NAME xml_parse_function_description_with_unknown_port
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/function_description_with_unknown_port.cpp
  USE_BOOST
  LIBRARIES pnet
            Boost::filesystem
)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/virtual_place_not_tunneled.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_not_tunneled.cpp
  @ONLY
)

fhg_add_test (NAME xml_parse_virtual_place_not_tunneled
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_not_tunneled.cpp
  USE_BOOST
  LIBRARIES pnet
            Boost::filesystem
)

fhg_add_test (NAME xml_parse_diagnostics
  SOURCES diagnostics.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/.."
  LIBRARIES pnet Boost::filesystem
  DESCRIPTION "check parser diagnostics"
)
