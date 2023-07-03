# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

include_guard()

###############################################################################
# Private
###############################################################################

string(ASCII 27 _util_cmake_esc)
set(_util_cmake_color_default      "${_util_cmake_esc}[m")
set(_util_cmake_color_red          "${_util_cmake_esc}[31m")
set(_util_cmake_color_green        "${_util_cmake_esc}[32m")
set(_util_cmake_color_yellow       "${_util_cmake_esc}[33m")
set(_util_cmake_color_blue         "${_util_cmake_esc}[34m")
set(_util_cmake_color_magenta      "${_util_cmake_esc}[35m")
set(_util_cmake_color_cyan         "${_util_cmake_esc}[36m")
set(_util_cmake_color_white        "${_util_cmake_esc}[37m")
set(_util_cmake_color_bold         "${_util_cmake_esc}[1m")
set(_util_cmake_color_bold_red     "${_util_cmake_esc}[1;31m")
set(_util_cmake_color_bold_green   "${_util_cmake_esc}[1;32m")
set(_util_cmake_color_bold_yellow  "${_util_cmake_esc}[1;33m")
set(_util_cmake_color_bold_blue    "${_util_cmake_esc}[1;34m")
set(_util_cmake_color_bold_magenta "${_util_cmake_esc}[1;35m")
set(_util_cmake_color_bold_cyan    "${_util_cmake_esc}[1;36m")
set(_util_cmake_color_bold_white   "${_util_cmake_esc}[1;37m")

###############################################################################
# Public
###############################################################################

#[============================================================================[
### Description:
This function colors a string using ANSI escape codes.

The colors can be disabled by either passing `-D UTIL_CMAKE_NO_COLORS=ON` to
CMake at configuration time or setting the environment variable
`UTIL_CMAKE_NO_COLORS=ON`. The environment variable will always take precedence
over the CMake definition.

The following colors are supported:

- default
- red
- green
- yellow
- blue
- magenta
- cyan
- white
- bold
- bold_red
- bold_green
- bold_yellow
- bold_blue
- bold_magenta
- bold_cyan
- bold_white

#### Command:
```cmake
util_cmake_color_string (
  <output-variable>
  <color>
  <string>
)
```
#]============================================================================]
function (util_cmake_color_string _output _color _string)
  if (DEFINED ENV{UTIL_CMAKE_NO_COLORS})
    set (UTIL_CMAKE_NO_COLORS $ENV{UTIL_CMAKE_NO_COLORS})
  endif()
  if (NOT UTIL_CMAKE_NO_COLORS)
    set (_string
      "${_util_cmake_color_${_color}}${_string}${_util_cmake_color_default}"
    )
  endif()
  set (${_output} "${_string}" PARENT_SCOPE)
endfunction()
