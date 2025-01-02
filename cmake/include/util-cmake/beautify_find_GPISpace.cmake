# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#"BEGIN INCLUDE"
include (util-cmake/parse_arguments)
#"END INCLUDE"

#! Wrap `find_package (GPISpace REQUIRED)` with extension to
#! - allow GPISpace and the current project being compiled with
#!   different versions of shared submodules by passing
#!   \a ALLOW_DIFFERENT_GIT_SUBMODULES.
#! - use the `GSPC_HOME` environment and CMake variable as
#!   `GPISpace_ROOT` for legacy compatibility
#! - instead of searching by \a VERSION, search by git \a
#!   REVISION. Note that this is only for compatibility with old
#!   versions of GPISpace. A GPISpace installation always only
#!   supports either version or revision.
#!
#! \deprecated Except for legacy compatibility, this function does not
#! provide anything in addition to `find_package()` itself:
#! - The equivalent to `REVISION xxx` is `COMPONENTS REVISION=xxx`.
#! - The equivalent for `ALLOW_DIFFERENT_GIT_SUBMODULES` is
#!   `COMPONENTS ALLOW_DIFFERENT_GIT_SUBMODULES` or `COMPONENTS
#!   DO_NOT_CHECK_GIT_SUBMODULES`.
#! - The equivalent for `VERSION x` is `x EXACT` in the version part
#!   of the `find_package()` call.
#! - There is no equivalent for `GSPC_HOME`, so users are advised to
#!   transform to the standard CMake variables `GPISpace_ROOT` or
#!   `CMAKE_PREFIX_PATH` and alike.
macro (find_GPISpace)
  set (options ALLOW_DIFFERENT_GIT_SUBMODULES)
  set (one_value_options REVISION VERSION)
  set (multi_value_options COMPONENTS)
  set (required_options)
  _parse_arguments (_find_gpispace_arg "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if (DEFINED ENV{GSPC_HOME})
    set (GPISpace_ROOT "$ENV{GSPC_HOME}")
  elseif (DEFINED GSPC_HOME)
    set (GPISpace_ROOT "${GSPC_HOME}")
  endif()

  if (_find_gpispace_arg_ALLOW_DIFFERENT_GIT_SUBMODULES)
    list (APPEND _find_gpispace_arg_COMPONENTS ALLOW_DIFFERENT_GIT_SUBMODULES)
  endif()

  if (NOT ALLOW_ANY_GPISPACE_VERSION)
    if (_find_gpispace_arg_REVISION)
      list (APPEND _find_gpispace_arg_COMPONENTS
        "REVISION=${_find_gpispace_arg_REVISION}")
    endif()

    set (_find_gpispace_version_arguments)
    if (_find_gpispace_arg_VERSION)
      set (_find_gpispace_version_arguments ${_find_gpispace_arg_VERSION} EXACT)
    endif()
  endif()

  find_package (GPISpace
    ${_find_gpispace_version_arguments}
    REQUIRED
    COMPONENTS ${_find_gpispace_arg_COMPONENTS})
endmacro()
