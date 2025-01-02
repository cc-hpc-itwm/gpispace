# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

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