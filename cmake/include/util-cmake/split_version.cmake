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

include_guard()

#[============================================================================[
### Description:
This function splits a given version string into list of its components MAJOR,
MINOR, PATCH, and TWEAK version, where each omitted component is defaulted to
0. The result is stored in a return variable.
In the case a version string contains more than these 4 components, it is
shrunk down to that size by removing any components past the fourth.

#### Command:
```cmake
util_cmake_split_version (
  <return-var>
  <version>
)
```
#]============================================================================]
function (util_cmake_split_version
  _return_var
  _version
)
  string (REPLACE "." ";" _version "${_version}")
  list (LENGTH _version _version_count)
  while (_version_count GREATER 4)
    list (POP_BACK _version)
    math (EXPR _version_count "${_version_count} - 1")
  endwhile()
  while (_version_count LESS_EQUAL 4)
    list (APPEND _version 0)
    math (EXPR _version_count "${_version_count} + 1")
  endwhile()

  set (${_return_var} ${_version} PARENT_SCOPE)
endfunction()
