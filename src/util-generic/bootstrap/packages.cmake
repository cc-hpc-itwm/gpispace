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

include_guard ()

include (${CMAKE_CURRENT_LIST_DIR}/bootstrap-macros.cmake)

#############################################################################
# Bootstrap Package Locations
#############################################################################

bootstrap_package (
  util-cmake
  URL http://gitlab.hpc.devnet.itwm.fhg.de/shared/cmake/-/archive/master/cmake-master.tar.gz
  VERSION 1.0.0
  VERSION_COMPATIBILITY ExactVersion
)
