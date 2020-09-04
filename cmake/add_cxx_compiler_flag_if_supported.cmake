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

include (CheckCXXCompilerFlag)
include (parse_arguments)

function (is_cxx_compiler_flag_supported _flag _result)
  set (${_result} FALSE PARENT_SCOPE)

  #! \note GCC accepts any -Wno- flag as valid flag for cross-version
  #! compatibility, but complains about it when any other error
  #! happened. Thus, for any warning we disable, check if the
  #! corresponding warning exists, and only then add the flag.
  string (LENGTH "${_flag}" _length)
  if (${_length} GREATER 5)
    string (SUBSTRING "${_flag}" 0 5 _prefix)
    string (SUBSTRING "${_flag}" 5 -1 _suffix)

    if ("${_prefix}" STREQUAL "-Wno-")
      is_cxx_compiler_flag_supported ("-W${_suffix}" _corresponding_supported)
      if (NOT ${_corresponding_supported})
        return()
      endif()
    endif()
  endif()

  string (MAKE_C_IDENTIFIER "CXX_COMPILER_SUPPORTS_${_flag}" _test_variable)
  check_cxx_compiler_flag ("${_flag}" ${_test_variable})
  if (${${_test_variable}})
    set (${_result} TRUE PARENT_SCOPE)
  endif()
endfunction()

macro (add_cxx_compiler_flag_if_supported _FLAG)
  is_cxx_compiler_flag_supported ("${_FLAG}" _is_supported)
  if (${_is_supported})
     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_FLAG}")
  endif()
endmacro()

function (add_cxx_compiler_flag_if_supported_source_files)
  set (options)
  set (one_value_options FLAG)
  set (multi_value_options SOURCES)
  #! \note SOURCES is not required since invoking with an empty list
  #! should be fine (and is), in generic code.
  set (required_options FLAG)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  is_cxx_compiler_flag_supported ("${ARG_FLAG}" _is_supported)
  if (${_is_supported})
    set_property (SOURCE ${ARG_SOURCES}
      APPEND_STRING
      PROPERTY COMPILE_FLAGS " ${ARG_FLAG}"
    )
  endif()
endfunction()
