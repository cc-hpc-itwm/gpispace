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

if (_type STREQUAL "option")
  message (WARNING
    "argument type OPTION ignores DEFAULT attribute"
  )
endif()

if (${_internal}_${_arg}_default)
  message (WARNING
    "${_arg}: multiple definitions of attribute DEFAULT"
  )
endif()

if (_arg IN_LIST ${_internal}_required)
  message (WARNING
    "DEFAULT attribute will be ignored when REQUIRED attribute is set"
  )
endif()

list (POP_FRONT ARGN _default)
set (${_internal}_${_arg}_default ${_default})

if (_type STREQUAL "multi_value" AND ARGN)
  list (GET ARGN 0 _next)
  string (TOLOWER "${_next}" _next)

  while (NOT _next IN_LIST ${_internal}_attributes AND NOT _next IN_LIST ${_internal}_types)
    list (POP_FRONT ARGN _default)
    list (APPEND ${_internal}_${_arg}_default ${_default})

    if (NOT ARGN)
      break()
    endif()

    list (GET ARGN 0 _next)
    string (TOLOWER "${_next}" _next)
  endwhile()
endif()
