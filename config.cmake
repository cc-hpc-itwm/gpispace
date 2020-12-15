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

set (CMAKE_BUILD_TYPE "Release" CACHE STRING "")
set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "")
set (FHG_ASSERT_MODE 1 CACHE BOOL "")
set (INSTALL_DO_NOT_BUNDLE ON CACHE BOOL "")
set (SHARED_DIRECTORY_FOR_TESTS "${CMAKE_BINARY_DIR}/shared_for_tests" CACHE PATH "")
file (MAKE_DIRECTORY "${SHARED_DIRECTORY_FOR_TESTS}")
set (TESTING_RIF_STRATEGY "ssh" CACHE STRING "")
set (CMAKE_FIND_PACKAGE_PREFER_CONFIG ON CACHE BOOL "")
