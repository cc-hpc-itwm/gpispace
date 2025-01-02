# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

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
