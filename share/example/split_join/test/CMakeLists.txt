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

set (REENTRANT_VERSION not_reentrant)

foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  set (SPLIT_JOIN_MAIN split_join.${SPLIT_VERSION}.${REENTRANT_VERSION})

  fhg_add_test (NAME share_example_${SPLIT_JOIN_MAIN}
    SOURCES test.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_BINARY_DIR}/../"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --main "${SPLIT_JOIN_MAIN}"
         --input 10000
         --expected-output 333383335000
    LIBRARIES gspc
              test-utilities
              fhg-util
              Boost::program_options
              Boost::filesystem
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endforeach()

set (REENTRANT_VERSION reentrant.sequential)

foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  set (SPLIT_JOIN_MAIN split_join.${SPLIT_VERSION}.${REENTRANT_VERSION})

  fhg_add_test (NAME share_example_${SPLIT_JOIN_MAIN}
    SOURCES test.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_BINARY_DIR}/../"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --main "${SPLIT_JOIN_MAIN}"
         --input 10000
         --expected-output 333383335000
         --input 20000
         --expected-output 2666866670000
    LIBRARIES gspc
              test-utilities
              fhg-util
              Boost::program_options
              Boost::filesystem
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endforeach()
