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
  gspc_add_library (NAME log_map-impl
    TYPE MODULE
    SOURCES "implementation.cpp"
    INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/share/doc/example"
    LIBRARIES GPISpace::header-only
              we-dev
  )

  math (EXPR size_input 50)
  math (EXPR size_output 50)
  math (EXPR size_block 10)
  math (EXPR num_block 10)
endif()

fhg_add_test (NAME doc_example_map_log
  SOURCES test.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/share/doc/example"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/map"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --implementation "$<TARGET_FILE:log_map-impl>"
       --size-input ${size_input}
       --size-output ${size_output}
       --size-block ${size_block}
       --num-block ${num_block}
       --user-data "y()"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  DEPENDS log_map-impl
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
