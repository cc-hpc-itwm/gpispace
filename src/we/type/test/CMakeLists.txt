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

fhg_add_test (NAME we_type_bytearray
  SOURCES bytearray.cpp
  USE_BOOST
  LIBRARIES pnet fhg-util
)

fhg_add_test (NAME we_type_value
  SOURCES value.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_signature
  SOURCES "${CMAKE_BINARY_DIR}/we/type/test/signature.cpp"
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_bitsetofint
  SOURCES bitsetofint.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_ModuleCall
  SOURCES ModuleCall.cpp
  USE_BOOST
  LIBRARIES pnet
)

gspc_add_executable (NAME mk_cpp
  SOURCES "mk_cpp.cpp"
  LIBRARIES pnet Util::Generic
)

fhg_add_test (NAME we_type_pnetc_compare
  SOURCES "pnetc_compare.cpp"
  USE_BOOST
  LIBRARIES pnet
)

add_custom_command (
  OUTPUT sig_struct.hpp sig_op.hpp sig_op.cpp
  COMMAND "$<TARGET_FILE:mk_cpp.exe>" -H sig_struct.hpp -O sig_op.hpp -I sig_op.cpp
)

gspc_add_library (NAME sig_op
  TYPE STATIC
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/sig_op.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES pnet
            Boost::base
)

fhg_add_test (NAME we_type_sig
  SOURCES sig.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES sig_op pnet
)

fhg_add_test (NAME we_type_get_schedule_data
  SOURCES get_schedule_data.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_dynamic_requirements
  SOURCES dynamic_requirements.cpp
  USE_BOOST
  LIBRARIES pnet we::test_equality
)

fhg_add_test (NAME we_type_transition_with_preferences
  SOURCES transition_with_preferences.cpp
  USE_BOOST
  LIBRARIES pnet we::test_equality
)

fhg_add_test (NAME we_type_MemoryBufferInfo
  SOURCES MemoryBufferInfo.cpp
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing
)

fhg_add_test (NAME we_type_memory_transfer
  SOURCES "memory_transfer.cpp"
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing
)

fhg_add_test (NAME we_type_Transition
  SOURCES "Transition.cpp"
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing
)
