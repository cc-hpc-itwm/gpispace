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
  gspc_add_library (NAME transform_file_type
    TYPE STATIC
    SOURCES "type.cpp"
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
    LIBRARIES we-dev
              Boost::filesystem
  )

  gspc_add_library (NAME transform_file_map-impl
    TYPE MODULE
    SOURCES "implementation.cpp"
    LIBRARIES GPISpace::header-only
              transform_file_type
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  )

  math (EXPR size_input "20 << 20")
  math (EXPR size_output "20 << 20")
  math (EXPR size_block "2 << 20")
  math (EXPR num_block "120")
endif()

fhg_add_test (NAME share_example_map_transform_file
  SOURCES test.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --implementation "$<TARGET_FILE:transform_file_map-impl>"
       --size-input ${size_input}
       --size-output ${size_output}
       --size-block ${size_block}
       --num-block ${num_block}
  LIBRARIES gspc
            test-utilities
            transform_file_type
            fhg-util
            Boost::program_options
            Boost::filesystem
  DEPENDS transform_file_map-impl
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
