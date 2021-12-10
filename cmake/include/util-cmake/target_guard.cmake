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

include_guard()

#[============================================================================[
### Description:
This macro provides a target definition guard for find_package module files. It
takes a list of target names and checks their existence. If all of them exist,
the find_package call returns and continues normally otherwise. In the case
where only some targets are defined, an error is triggered.

#### Command:
```cmake
util_cmake_target_guard (
  <target>...
)
```
#]============================================================================]
macro (util_cmake_target_guard)
  set (_expected_targets ${ARGN})
  unset (_defined_targets)
  unset (_undefined_targets)
  foreach (_target ${_expected_targets})
    if (TARGET ${_target})
      list (APPEND
        _defined_targets
        ${_target}
      )
    else()
      list (APPEND
        _undefined_targets
        ${_target}
      )
    endif()
  endforeach()
  if (_defined_targets STREQUAL _expected_targets)
    unset (_expected_targets)
    unset (_defined_targets)
    unset (_undefined_targets)
    return()
  elseif (NOT "${_defined_targets}" STREQUAL "")
    message (FATAL_ERROR
      "Some (but not all) targets in this package were already defined.\nTargets Defined: ${_defined_targets}\nTargets not yet defined: ${_undefined_targets}\n"
    )
  endif()
  unset (_expected_targets)
  unset (_defined_targets)
  unset (_undefined_targets)
endmacro()
