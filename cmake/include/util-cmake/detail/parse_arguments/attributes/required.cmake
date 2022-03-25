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

if (_arg IN_LIST ${_internal}_required)
  message (WARNING
    "${_arg}: multiple definitions of attribute REQUIRED"
  )
else()
  list (APPEND ${_internal}_required ${_arg})
endif()

if (${_internal}_${_arg}_default)
  message (WARNING
    "DEFAULT attribute will be ignored when REQUIRED attribute is set"
  )
endif()
