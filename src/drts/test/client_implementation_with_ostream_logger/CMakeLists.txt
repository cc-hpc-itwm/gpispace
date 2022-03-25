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

if (BUILD_TESTING)
  gspc_add_library (NAME client_implementation_with_ostream_logger
    SOURCES "client_implementation_with_ostream_logger.cpp"
    TYPE MODULE
    LIBRARIES GPISpace::header-only
  )
endif()

fhg_add_test (NAME drts_client_implementation_with_ostream_logger
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --implementation "$<TARGET_FILE:client_implementation_with_ostream_logger>"
  LIBRARIES gspc
            test-utilities
            fhg-util
            gspc::logging
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
