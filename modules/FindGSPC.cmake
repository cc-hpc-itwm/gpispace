# -*- mode: cmake; -*-
# This file defines:
# * GSPC_FOUND   if the gspc library
# * GSPC_LIBRARY the gspc library
# * GSPC_INCLUDE_DIR The path to the include directory

if (NOT TARGET libgspc)
  find_path (GSPC_INCLUDE_DIR
    NAMES "gspc/gspc.hpp"
    HINTS ${GSPC_HOME} ENV GSPC_HOME
    PATH_SUFFIXES include
    )
  set (GSPC_INCLUDE_DIR "${GSPC_INCLUDE_DIR}" CACHE FILEPATH "gspc include dir")

  find_library (GSPC_LIBRARY_STATIC
    NAMES libgspc.a
    HINTS ${GSPC_HOME} ENV GSPC_HOME
    PATH_SUFFIXES lib
    )
  set (GSPC_LIBRARY_STATIC "${GSPC_LIBRARY_STATIC}" CACHE FILEPATH "gspc static library")

  find_library (GSPC_LIBRARY_SHARED
    NAMES libgspc.so libgspc.dylib
    HINTS ${GSPC_HOME} ENV GSPC_HOME
    PATH_SUFFIXES lib
    )
  set (GSPC_LIBRARY_SHARED "${GSPC_LIBRARY_SHARED}" CACHE FILEPATH "gspc shared library")

  if (GSPC_LIBRARY_STATIC)
    set (GSPC_LIBRARY "${GSPC_LIBRARY_STATIC}" CACHE FILEPATH "default gspc library")
  elseif (GSPC_LIBRARY_SHARED)
    set (GSPC_LIBRARY "${GSPC_LIBRARY_SHARED}" CACHE FILEPATH "default gspc library")
  endif ()

  if (GSPC_INCLUDE_DIR AND GSPC_LIBRARY)
    set (GSPC_FOUND TRUE)
    if (NOT GSPC_FIND_QUIETLY)
      message(STATUS "Found GSPC headers in ${GSPC_INCLUDE_DIR}")
      if (GSPC_LIBRARY_STATIC)
	message(STATUS "Found GSPC static library in ${GSPC_LIBRARY}")
      endif ()
      if (GSPC_LIBRARY_SHARED)
	message(STATUS "Found GSPC shared library in ${GSPC_LIBRARY_SHARED}")
      endif ()
    endif ()
  else ()
    if (GSPC_FIND_REQUIRED)
      message (FATAL_ERROR "GSPC could not be found!")
    endif ()
  endif ()
else ()
  set (GSPC_FOUND true)
  set (GSPC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gspc;${CMAKE_BINARY_DIR}/gspc" CACHE STRING "gspc include dir")

  set (GSPC_LIBRARY_STATIC libgspc CACHE STRING "gspc static library")
  set (GSPC_LIBRARY_SHARED libgspc.shared CACHE STRING "gspc shared library")
  set (GSPC_LIBRARY "${GSPC_LIBRARY_STATIC}" CACHE STRING "gspc default library")

  if (NOT GSPC_FIND_QUIETLY)
    message(STATUS "Found GSPC headers in ${GSPC_INCLUDE_DIR}")
    if (GSPC_LIBRARY_STATIC)
      message(STATUS "Found GSPC static library in ${GSPC_LIBRARY}")
    endif ()
    if (GSPC_LIBRARY_SHARED)
      message(STATUS "Found GSPC shared library in ${GSPC_LIBRARY_SHARED}")
    endif ()
  endif ()
endif ()

mark_as_advanced (FORCE
  GSPC_LIBRARY
  GSPC_LIBRARY_STATIC
  GSPC_LIBRARY_SHARED
  GSPC_INCLUDE_DIR
)
