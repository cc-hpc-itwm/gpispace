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

if (BUILD_TESTING)
  gspc_add_library (NAME Log
    NAMESPACE Plugin
    SOURCES "Log.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
  gspc_add_library (NAME Tunnel
    NAMESPACE Plugin
    SOURCES "Tunnel.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
endif()

function (test_Log case)
  fhg_add_test (NAME plugin_Log_${case}
    SOURCES test/Log/${case}/${case}.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/test/Log/${case}"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --plugin-path "$<TARGET_FILE:Plugin::Log>"
    LIBRARIES test-utilities
              fhg-util
              Boost::program_options
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endfunction()

test_Log ("echo")
test_Log ("create_and_call_after_in_one_transition")
test_Log ("call_before_and_destroy_in_one_transition")

function (test_Tunnel case)
  fhg_add_test (NAME plugin_Tunnel_${case}
    SOURCES test/Tunnel/${case}/${case}.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/test/Tunnel/${case}"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --plugin-path "$<TARGET_FILE:Plugin::Tunnel>"
    LIBRARIES test-utilities
              fhg-util
              Boost::program_options
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endfunction()

test_Tunnel (subnetwork)
