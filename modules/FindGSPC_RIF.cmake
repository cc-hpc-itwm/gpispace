# -*- mode: cmake; -*-
# This file defines:
# * GSPC_RIF_FOUND   if the gspcrif library
# * GSPC_RIF_LIBRARY the gspcrif library
# * GSPC_RIF_INCLUDE_DIR The path to the include directory
if (NOT GSPC_RIF_FIND_QUIETLY)
  message(STATUS "FindGSPC_RIF check")
endif ()

if (NOT TARGET gspcrif)
  find_path (GSPC_RIF_INCLUDE_DIR
    NAMES "gspc/rif.hpp"
    HINTS ${GSPC_RIF_HOME} ENV GSPC_RIF_HOME
    PATH_SUFFIXES include
    )
  set (GSPC_RIF_INCLUDE_DIR "${GSPC_RIF_INCLUDE_DIR}" CACHE FILEPATH "gspcrif include dir")

  find_library (GSPC_RIF_LIBRARY_STATIC
    NAMES libgspcrif.a
    HINTS ${GSPC_RIF_HOME} ENV GSPC_RIF_HOME
    PATH_SUFFIXES lib
    )
  set (GSPC_RIF_LIBRARY_STATIC "${GSPC_RIF_LIBRARY_STATIC}" CACHE FILEPATH "gspcrif static library")

  find_library (GSPC_RIF_LIBRARY_SHARED
    NAMES libgspcrif.so libgspcrif.dylib
    HINTS ${GSPC_RIF_HOME} ENV GSPC_RIF_HOME
    PATH_SUFFIXES lib
    )
  set (GSPC_RIF_LIBRARY_SHARED "${GSPC_RIF_LIBRARY_SHARED}" CACHE FILEPATH "gspcrif shared library")

  if (GSPC_RIF_LIBRARY_STATIC)
    set (GSPC_RIF_LIBRARY "${GSPC_RIF_LIBRARY_STATIC}" CACHE FILEPATH "default gspcrif library")
  elseif (GSPC_RIF_LIBRARY_SHARED)
    set (GSPC_RIF_LIBRARY "${GSPC_RIF_LIBRARY_SHARED}" CACHE FILEPATH "default gspcrif library")
  endif ()

  if (GSPC_RIF_INCLUDE_DIR AND GSPC_RIF_LIBRARY)
    set (GSPC_RIF_FOUND TRUE)
    if (NOT GSPC_RIF_FIND_QUIETLY)
      message(STATUS "Found GSPC_RIF headers in ${GSPC_RIF_INCLUDE_DIR}")
      if (GSPC_RIF_LIBRARY_STATIC)
	message(STATUS "Found GSPC_RIF static library in ${GSPC_RIF_LIBRARY}")
      endif ()
      if (GSPC_RIF_LIBRARY_SHARED)
	message(STATUS "Found GSPC_RIF shared library in ${GSPC_RIF_LIBRARY_SHARED}")
      endif ()
    endif ()
  else ()
    if (GSPC_RIF_FIND_REQUIRED)
      message (FATAL_ERROR "GSPC_RIF could not be found!")
    endif ()
  endif ()
else ()
  set (GSPC_RIF_FOUND true)
  set (GSPC_RIF_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gspcrif;${CMAKE_BINARY_DIR}/gspcrif" CACHE STRING "gspcrif include dir")

  set (GSPC_RIF_LIBRARY_STATIC gspcrif CACHE STRING "gspcrif static library")
  set (GSPC_RIF_LIBRARY_SHARED gspcrif.shared CACHE STRING "gspcrif shared library")
  set (GSPC_RIF_LIBRARY "${GSPC_RIF_LIBRARY_STATIC}" CACHE STRING "gspcrif default library")

  if (NOT GSPC_RIF_FIND_QUIETLY)
    message(STATUS "Found GSPC_RIF headers in ${GSPC_RIF_INCLUDE_DIR}")
    if (GSPC_RIF_LIBRARY_STATIC)
      message(STATUS "Found GSPC_RIF static library in ${GSPC_RIF_LIBRARY}")
    endif ()
    if (GSPC_RIF_LIBRARY_SHARED)
      message(STATUS "Found GSPC_RIF shared library in ${GSPC_RIF_LIBRARY_SHARED}")
    endif ()
  endif ()
endif ()

mark_as_advanced (FORCE
  GSPC_RIF_LIBRARY
  GSPC_RIF_LIBRARY_STATIC
  GSPC_RIF_LIBRARY_SHARED
  GSPC_RIF_INCLUDE_DIR
)
