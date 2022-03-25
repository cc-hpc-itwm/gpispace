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

set (_arg "HELP")
set (${_internal}_${_type} ON)
list (APPEND ${_internal}_options ${_arg})

if (${_internal}_${_arg}_description)
  message (WARNING
    "${_arg}: multiple definitions of attribute DESCRIPTION"
  )
else()
  set (${_internal}_${_arg}_description "Displays this message")
endif()
