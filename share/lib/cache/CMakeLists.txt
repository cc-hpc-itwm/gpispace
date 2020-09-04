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

install (FILES
  maybe_start_to_fill.xpnet
  done_with_slot.xpnet
  fill.xpnet
  get_slot_for_id.xpnet
  init.xpnet
  start_to_fill.xpnet
  type.xpnet
  DESTINATION ${PNETC_LIB_DIR}/cache
  )

add_subdirectory (demo)
