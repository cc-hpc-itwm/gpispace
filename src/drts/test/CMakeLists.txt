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

fhg_add_test (NAME drts_installation
  SOURCES installation.cpp
  USE_BOOST
  LIBRARIES gspc
            Boost::program_options
  REQUIRES_INSTALLATION
)
fhg_add_test (NAME drts_installation_from_command_line
  SOURCES installation_from_command_line.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
  LIBRARIES gspc
            Boost::program_options
  REQUIRES_INSTALLATION
)

fhg_add_test (NAME drts_context
  SOURCES context.cpp
  USE_BOOST
  LIBRARIES gspc
)

fhg_add_test (NAME scoped_rifd
  SOURCES scoped_rifd.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

fhg_add_test (NAME scoped_drts
  SOURCES scoped_drts.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

extended_add_library (NAME module_nounload_detection-empty
  SOURCES "module_nounload_detection/empty.cpp"
  TYPE SHARED
)

fhg_add_test (NAME module_nounload_detection
  SOURCES "module_nounload_detection.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Util::Generic
            Boost::filesystem
            Boost::program_options
  COMPILE_DEFINITIONS
    PRIVATE "LIBRARY_TO_LOAD=\"$<TARGET_FILE:module_nounload_detection-empty>\""
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  DEPENDS module_nounload_detection-empty
)

fhg_add_test (NAME worker_env
  SOURCES "worker_env.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Util::Generic
            Boost::filesystem
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

add_subdirectory (client_implementation_with_ostream_logger)
add_subdirectory (scoped_rifd_execute)

fhg_add_test (NAME iml_to_pnet_value_wrappers
  SOURCES "iml_to_pnet_value_wrappers.cpp"
  USE_BOOST
  LIBRARIES gspc
            Util::Generic
            # \todo Is this fine? There is no public API to generate
            # random handles... Remove randomness?
            IMLPrivate::Testing
)
