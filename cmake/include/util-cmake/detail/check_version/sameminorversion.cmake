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

if (_base_version VERSION_LESS_EQUAL _given_version)
  util_cmake_split_version (_base_version ${_base_version})
  list (GET _base_version 0 _base_major)
  list (GET _base_version 1 _base_minor)

  util_cmake_split_version (_given_version ${_given_version})
  list (GET _given_version 0 _given_major)
  list (GET _given_version 1 _given_minor)

  if (_base_major EQUAL _given_major AND _base_minor EQUAL _given_minor)
    set (_result true)
  endif()
endif()
