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

# locates the librt library
# This file defines:
# * LIBRT_FOUND if librt was found
# * LIBRT_LIBRARY The lib to link to (currently only a static unix lib)
# * LIBRT_STATIC_LIBRARY
# * LIBRT_SHARED_LIBRARY The lib to link to (currently only a static unix lib)
# * LIBRT_INCLUDE_DIR

if (NOT LibRT_FIND_QUIETLY)
  message(STATUS "FindLibRT check")
endif ()

find_path (LIBRT_INCLUDE_DIR
  NAMES "sys/mman.h"
  PATH_SUFFIXES include
  )

find_library (LIBRT_STATIC_LIBRARY
  NAMES librt.a
  PATH_SUFFIXES lib lib64
  )
find_library (LIBRT_SHARED_LIBRARY
  NAMES librt.so
  PATH_SUFFIXES lib lib64
  )

if (LIBRT_STATIC_LIBRARY)
  set(LIBRT_LIBRARY ${LIBRT_STATIC_LIBRARY})
else()
  if (LIBRT_SHARED_LIBRARY)
    set(LIBRT_LIBRARY ${LIBRT_SHARED_LIBRARY})
  endif()
endif()

if (LIBRT_INCLUDE_DIR AND LIBRT_LIBRARY)
  set (LIBRT_FOUND TRUE)
  if (NOT LibRT_FIND_QUIETLY)
    message (STATUS "Found librt headers in ${LIBRT_INCLUDE_DIR} and libraries ${LIBRT_STATIC_LIBRARY} ${HWLOC_SHARED_LIBRARY}")
  endif ()
else ()
  if (LibRT_FIND_REQUIRED)
    message (FATAL_ERROR "librt could not be found!")
  endif ()
endif ()
