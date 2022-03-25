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
