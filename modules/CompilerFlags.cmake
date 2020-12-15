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

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED on)

macro (CHECK_AND_ADD_COMPILER_FLAG _VAR _FLAG)
  STRING(REGEX REPLACE "[-=]" "_" __flag_literal ${_FLAG})
  set(__flag_literal "FLAG${__flag_literal}")
  CHECK_CXX_COMPILER_FLAG (${_FLAG} ${__flag_literal})
  if (${__flag_literal})
     set (${_VAR} "${${_VAR}} ${_FLAG}")
  endif ()
endmacro ()

CHECK_AND_ADD_COMPILER_FLAG (CMAKE_CXX_FLAGS -W)
CHECK_AND_ADD_COMPILER_FLAG (CMAKE_CXX_FLAGS -Wall)
CHECK_AND_ADD_COMPILER_FLAG (CMAKE_CXX_FLAGS -Wextra)
check_and_add_compiler_flag (CMAKE_CXX_FLAGS -Wunreachable-code-break)
check_and_add_compiler_flag (CMAKE_CXX_FLAGS -Wimplicit-fallthrough)
CHECK_AND_ADD_COMPILER_FLAG (CMAKE_CXX_FLAGS -Wnon-virtual-dtor)
CHECK_AND_ADD_COMPILER_FLAG (CMAKE_CXX_FLAGS -fpic)
CHECK_AND_ADD_COMPILER_FLAG (CMAKE_CXX_FLAGS -fPIC)
CHECK_AND_ADD_COMPILER_FLAG (CMAKE_CXX_FLAGS -ftemplate-depth=1024)
CHECK_AND_ADD_COMPILER_FLAG (CMAKE_CXX_FLAGS -fcolor-diagnostics)
