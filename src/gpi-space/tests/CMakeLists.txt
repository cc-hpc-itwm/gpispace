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

fhg_add_test (NAME vmem_handle_generator
  SOURCES handle_generator.cpp
  USE_BOOST
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES gpi-space-pc-segment
            gpi-space-pc-memory
            fhgcom
            ${CMAKE_THREAD_LIBS_INIT}
            fhg-util
)

fhg_add_test (NAME vmem_memory_area
  SOURCES memory_area.cpp
  USE_BOOST
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES gpi-space-pc-segment
            gpi-space-pc-memory
            fhgcom
            ${CMAKE_THREAD_LIBS_INIT}
            fhg-util
)

fhg_add_test (NAME vmem_segment
  SOURCES segment.cpp
  USE_BOOST
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES gpi-space-pc-segment
            gpi-space-pc-memory
            fhgcom
            ${CMAKE_THREAD_LIBS_INIT}
            fhg-util
)

extended_add_executable (NAME check_beegfs_segment_requirements
  SOURCES "check_beegfs_segment_requirements.cpp"
  LIBRARIES GPISpace::vmem
            Util::Generic
            test-utilities
            Boost::program_options
)

file (GENERATE OUTPUT "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
  CONTENT "execute_process (COMMAND \"$<TARGET_FILE:check_beegfs_segment_requirements.exe>\"
                                    \"${SHARED_DIRECTORY_FOR_TESTS}\"
             WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\"
             RESULT_VARIABLE check_beegfs_segment_requirements_result
           )

           if (NOT \${check_beegfs_segment_requirements_result} EQUAL 0)
             list (APPEND CTEST_CUSTOM_TESTS_IGNORE vmem_beegfs_area)
           endif()"
)

fhg_add_test (NAME vmem_beegfs_area
  SOURCES beegfs_area.cpp
  USE_BOOST
  ARGS --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES gpi-space-pc-segment
            gpi-space-pc-memory
            fhgcom
            Boost::filesystem
            ${CMAKE_THREAD_LIBS_INIT}
            fhg-util
            test-utilities
)
