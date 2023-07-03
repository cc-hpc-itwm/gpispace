# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#"BEGIN INCLUDE"
include (util-cmake/parse_arguments)
#"END INCLUDE"

#! Require that if the C++ compiler \a COMPILER is used, it is used in
#! version >= \a VERSION.
#! \note `cmake --help-variable "CMAKE_<LANG>_COMPILER_ID"` gives a
#! list of possible compiler ids.
function (require_compiler_version_ge)
  set (options)
  set (one_value_options COMPILER VERSION)
  set (multi_value_options)
  set (required_options COMPILER VERSION)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if (CMAKE_CXX_COMPILER_ID STREQUAL "${ARG_COMPILER}")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "${ARG_VERSION}")
      message (FATAL_ERROR "${PROJECT_NAME} requires compiler ${CMAKE_CXX_COMPILER_ID} to be of version >= ${ARG_VERSION} (got ${CMAKE_CXX_COMPILER_VERSION}).")
    endif()
  endif()
endfunction()
