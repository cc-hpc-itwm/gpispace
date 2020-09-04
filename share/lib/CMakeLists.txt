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
  4.xml
  5.xml
  6.xml
  dup.xml
  eatN.xml
  make_pair.xml
  replicate.xpnet
  sequence_bounded.xml
  sequence.xml
  sequence_control.xml
  tagged_sequence_bounded.xml
  tagged_sequence.xml
  trigger_if.xml
  trigger_when.xml
  triple.xml
  wait.xml
  DESTINATION ${PNETC_LIB_DIR}
  )

add_subdirectory (cache)
add_subdirectory (grid)
add_subdirectory (memory)
add_subdirectory (point)
add_subdirectory (sequence)
add_subdirectory (stream)
