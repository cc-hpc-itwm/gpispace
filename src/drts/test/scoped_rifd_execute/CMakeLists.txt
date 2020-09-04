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

extended_add_executable (NAME test_binary
  SOURCES "test_binary.cpp"
  LIBRARIES rif-started_process_promise
            Util::Generic
            Boost::program_options
)
_create_bundle (test_binary.exe BUNDLE_PATH)

fhg_add_test (NAME drts_scoped_rifd_execute
  SOURCES scoped_rifd_execute.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --test-binary "$<TARGET_FILE:test_binary.exe>"
       --bundle-path "${BUNDLE_PATH}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
 REQUIRES_INSTALLATION
 START_SCOPED_RIF
)
