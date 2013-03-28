# -*- mode: cmake; -*-
# This file defines:
# * GPI_SPACE_FOUND   if the gpi process container api was found
# * GPI_SPACE_LIBRARY the gpi process container api library
# * GPI_SPACE_INCLUDE_DIR The path to the include directory
if (NOT GPISpace_FIND_QUIETLY)
  message(STATUS "FindGPISpace check")
endif (NOT GPISpace_FIND_QUIETLY)

if (NOT TARGET gpi-space-pc-client-static)
  find_path (GPI_SPACE_INCLUDE_DIR
    NAMES "gpi-space/pc/client/api.hpp"
    HINTS ${GPI_SPACE_HOME} ENV GPI_SPACE_HOME
    PATH_SUFFIXES include
    )

  find_library (GPI_SPACE_PC_CLIENT_LIBRARY
    NAMES libgpi-space-pc-client.a
    HINTS ${GPI_SPACE_HOME} ENV GPI_SPACE_HOME
    PATH_SUFFIXES lib
    )
  find_library (GPI_SPACE_PC_CLIENT_LIBRARY_SHARED
    NAMES libgpi-space-pc-client.so libgpi-space-pc-client.dylib
    HINTS ${GPI_SPACE_HOME} ENV GPI_SPACE_HOME
    PATH_SUFFIXES lib
    )

  find_library (GPI_SPACE_PC_SEGMENT_LIBRARY
    NAMES libgpi-space-pc-segment.a
    HINTS ${GPI_SPACE_HOME} ENV GPI_SPACE_HOME
    PATH_SUFFIXES lib
    )
  find_library (GPI_SPACE_PC_SEGMENT_LIBRARY_SHARED
    NAMES libgpi-space-pc-segment.so libgpi-space-pc-segment.dylib
    HINTS ${GPI_SPACE_HOME} ENV GPI_SPACE_HOME
    PATH_SUFFIXES lib
    )

  set(GPI_SPACE_LIBRARY
    ${GPI_SPACE_PC_CLIENT_LIBRARY}
    ${GPI_SPACE_PC_SEGMENT_LIBRARY}
    )
  set(GPI_SPACE_LIBRARY_SHARED
    ${GPI_SPACE_PC_CLIENT_LIBRARY_SHARED}
    ${GPI_SPACE_PC_SEGMENT_LIBRARY_SHARED}
    )

  if (GPI_SPACE_INCLUDE_DIR)
    set (GPI_SPACE_FOUND TRUE)
    if (NOT GPISpace_FIND_QUIETLY)
      message(STATUS "Found GPISpace headers in ${GPI_SPACE_INCLUDE_DIR}")
      if (GPI_SPACE_LIBRARY)
	message(STATUS "Found GPISpace static library in ${GPI_SPACE_LIBRARY}")
      endif (GPI_SPACE_LIBRARY)
      if (GPI_SPACE_LIBRARY_SHARED)
	message(STATUS "Found GPISpace shared library in ${GPI_SPACE_LIBRARY_SHARED}")
      endif (GPI_SPACE_LIBRARY_SHARED)
    endif (NOT GPISpace_FIND_QUIETLY)
  else (GPI_SPACE_INCLUDE_DIR)
    if (GPI_SPACE_FIND_REQUIRED)
      message (FATAL_ERROR "GPISpace could not be found!")
    endif (GPI_SPACE_FIND_REQUIRED)
  endif (GPI_SPACE_INCLUDE_DIR)
else (NOT TARGET gpi-space-pc-client-static)
  set(GPI_SPACE_FOUND true)
  set(GPI_SPACE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gpi-space;${CMAKE_BINARY_DIR}/gpi-space")

  set(GPI_SPACE_LIBRARY gpi-space-pc-client-static gpi-space-pc-segment)
  set(GPI_SPACE_LIBRARY_SHARED gpi-space-pc-client-shared gpi-space-pc-segment-shared)

  if (NOT GPISpace_FIND_QUIETLY)
    message(STATUS "Found GPISpace headers in ${GPI_SPACE_INCLUDE_DIR}")
    if (GPI_SPACE_LIBRARY)
      message(STATUS "Found GPISpace static library in ${GPI_SPACE_LIBRARY}")
    endif (GPI_SPACE_LIBRARY)
    if (GPI_SPACE_LIBRARY_SHARED)
      message(STATUS "Found GPISpace shared library in ${GPI_SPACE_LIBRARY_SHARED}")
    endif (GPI_SPACE_LIBRARY_SHARED)
  endif (NOT GPISpace_FIND_QUIETLY)
endif(NOT TARGET gpi-space-pc-client-static)
