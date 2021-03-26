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

include (parse_arguments)

macro (find_GPISpace)
  cmake_minimum_required (VERSION 3.12)

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
