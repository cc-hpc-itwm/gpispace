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

#"BEGIN GUARD"
include_guard()
#"END GUARD"

#[[
### Description:
Determines if a project has been called as the root project.
The output variable is set to `TRUE` in that case, otherwise to `FALSE`.

#### Command:
```cmake
util_cmake_is_project_root (
  <output-variable>
)
```
#]]
function (util_cmake_is_project_root _output)
  if ("${PROJECT_NAME}" STREQUAL "${CMAKE_PROJECT_NAME}")
    set (${_output} true PARENT_SCOPE)
  else ()
    set (${_output} false PARENT_SCOPE)
  endif ()
endfunction ()

#[[
### Description:
Returns the current project's name (i.e. `${PROJECT_NAME}`) as an all uppercase C identifier string.
This string is primarily meant to be used as a prefix for project specific CMakeCache entries.

#### Command:
```cmake
util_cmake_project_entry_name (
  <output-variable>
)
```
#]]
function (util_cmake_project_entry_name _output)
  string (TOUPPER "${PROJECT_NAME}" _entry_name)
  string (MAKE_C_IDENTIFIER "${_entry_name}" _entry_name)
  set (${_output} ${_entry_name} PARENT_SCOPE)
endfunction ()
