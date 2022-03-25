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

###############################################################################
# Includes
###############################################################################

set (_include_dir ${CMAKE_CURRENT_LIST_DIR})

include (${_include_dir}/split_version.cmake)

###############################################################################
# Private
###############################################################################

get_filename_component (_filename
  ${CMAKE_CURRENT_LIST_FILE}
  NAME_WLE
)

###############################################################################
# Public
###############################################################################

#[============================================================================[
### Description:
This function checks to version strings against each other using a
compatibility scheme and storing the result in a return variable.
Valid compatibility schemes are:

- AnyNewerVersion
- SameMajorVersion
- SameMinorVersion
- ExactVersion

#### Command:
```cmake
util_cmake_check_version (
  <compatibility>
  <return-var>
  <base-version>
  <given-version>
)
```

The `<given-version>` needs to fulfill the `<compatibility>` requirements
relative to the `<base-version>`.
#]============================================================================]
function (util_cmake_check_version
  _compatibility
  _return_var
  _base_version
  _given_version
)
  set (_detail_dir "${_include_dir}/detail/${_filename}")

  set (_result false)
  string (TOLOWER "${_compatibility}" _compatibility)
  include (${_detail_dir}/${_compatibility}.cmake)

  set (${_return_var} ${_result} PARENT_SCOPE)
endfunction()
