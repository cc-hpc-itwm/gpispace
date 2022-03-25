# This file is part of GPI-Space.
# Copyright (C) 2022 Fraunhofer ITWM
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

set (XML_PARSE_TEST_XPNETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xpnets")

configure_file (parser_fixture.hpp.in parser_fixture.hpp @ONLY)

fhg_add_test (NAME xml_parse_duplicate_connect
  SOURCES parse_duplicate_connect.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES pnet Boost::filesystem
  DESCRIPTION "pnetc shall throw if there are mutiple connections with the same port, place and direction"
)

fhg_add_test (NAME xml_util_position
  SOURCES util_position.cpp
  USE_BOOST
  LIBRARIES pnet Boost::filesystem
  DESCRIPTION "test xml_util_position"
)

fhg_add_test (NAME xml_memory_buffer
  SOURCES memory_buffer.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME xml_memory_transfer
  SOURCES memory_transfer.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME xml_parse_memory_buffer
  SOURCES parse_memory_buffer.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME xml_parse_memory_transfer
  SOURCES parse_memory_transfer.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME xml_conditions_and_expressions_with_cdata
  SOURCES conditions_and_expressions_with_cdata.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME xml_parse_invalid_preference_list
  SOURCES parse_invalid_preference_list.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME xml_parse_valid_preference_list
  SOURCES parse_valid_preference_list.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME xml_parse_invalid_eureka
  SOURCES parse_invalid_eureka.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES pnet Boost::filesystem
  DESCRIPTION "pnetc shall throw for mutiple eureka connections with the same port and no output port for eureka"
)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/virtual_place_with_token_can_be_synthesized.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_with_token_can_be_synthesized.cpp
  @ONLY
)

fhg_add_test (NAME xml_parse_virtual_place_with_token_can_be_synthesized
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_with_token_can_be_synthesized.cpp
  USE_BOOST
  LIBRARIES pnet
            Util::Generic
            Boost::filesystem
)

fhg_add_test (NAME xml_module_pass_context
  SOURCES module_pass_context.cpp
  USE_BOOST
  LIBRARIES pnet
)

add_subdirectory (bytearray)
add_subdirectory (diagnostics)
add_subdirectory (template_with_module_call)
