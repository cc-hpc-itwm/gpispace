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

set (SPLIT_JOIN_SPLIT_VERSIONS
  join_after_split
  parallel_split_join
)

set (SPLIT_JOIN_REENTRANT_VERSIONS
  not_reentrant
  reentrant.sequential
)

foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  foreach (REENTRANT_VERSION ${SPLIT_JOIN_REENTRANT_VERSIONS})
    configure_file (
      ${CMAKE_CURRENT_SOURCE_DIR}/split_join.${REENTRANT_VERSION}.xpnet.in
      ${CMAKE_CURRENT_BINARY_DIR}/split_join.${SPLIT_VERSION}.${REENTRANT_VERSION}.xpnet
      @ONLY
    )
  endforeach()

  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/split.${SPLIT_VERSION}.xpnet
    ${CMAKE_CURRENT_BINARY_DIR}/split.${SPLIT_VERSION}.xpnet
    COPYONLY
  )
endforeach()

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/join.xpnet
  ${CMAKE_CURRENT_BINARY_DIR}/join.xpnet
  COPYONLY
)

add_subdirectory (test)
