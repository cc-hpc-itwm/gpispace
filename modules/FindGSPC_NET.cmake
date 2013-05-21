# -*- mode: cmake; -*-
# This file defines:
# * GSPC_NET_FOUND   if the gspcnet library
# * GSPC_NET_LIBRARY the gspcnet library
# * GSPC_NET_INCLUDE_DIR The path to the include directory
if (NOT GSPC_NET_FIND_QUIETLY)
  message(STATUS "FindGSPC_NET check")
endif ()

if (NOT TARGET gspcnet)
  find_path (GSPC_NET_INCLUDE_DIR
    NAMES "gspc/net.hpp"
    HINTS ${GSPC_NET_HOME} ENV GSPC_NET_HOME
    PATH_SUFFIXES include
    )
  set (GSPC_NET_INCLUDE_DIR "${GSPC_NET_INCLUDE_DIR}" CACHE FILEPATH "gspcnet include dir")

  find_library (GSPC_NET_LIBRARY_STATIC
    NAMES libgspcnet.a
    HINTS ${GSPC_NET_HOME} ENV GSPC_NET_HOME
    PATH_SUFFIXES lib
    )
  set (GSPC_NET_LIBRARY_STATIC "${GSPC_NET_LIBRARY_STATIC}" CACHE FILEPATH "gspcnet static library")

  find_library (GSPC_NET_LIBRARY_SHARED
    NAMES libgspcnet.so libgspcnet.dylib
    HINTS ${GSPC_NET_HOME} ENV GSPC_NET_HOME
    PATH_SUFFIXES lib
    )
  set (GSPC_NET_LIBRARY_SHARED "${GSPC_NET_LIBRARY_SHARED}" CACHE FILEPATH "gspcnet shared library")

  if (GSPC_NET_LIBRARY_STATIC)
    set (GSPC_NET_LIBRARY "${GSPC_NET_LIBRARY_STATIC}" CACHE FILEPATH "default gspcnet library")
  elseif (GSPC_NET_LIBRARY_SHARED)
    set (GSPC_NET_LIBRARY "${GSPC_NET_LIBRARY_SHARED}" CACHE FILEPATH "default gspcnet library")
  endif ()

  if (GSPC_NET_INCLUDE_DIR AND GSPC_NET_LIBRARY)
    set (GSPC_NET_FOUND TRUE)
    if (NOT GSPC_NET_FIND_QUIETLY)
      message(STATUS "Found GSPC_NET headers in ${GSPC_NET_INCLUDE_DIR}")
      if (GSPC_NET_LIBRARY_STATIC)
	message(STATUS "Found GSPC_NET static library in ${GSPC_NET_LIBRARY}")
      endif ()
      if (GSPC_NET_LIBRARY_SHARED)
	message(STATUS "Found GSPC_NET shared library in ${GSPC_NET_LIBRARY_SHARED}")
      endif ()
    endif ()
  else ()
    if (GSPC_NET_FIND_REQUIRED)
      message (FATAL_ERROR "GSPC_NET could not be found!")
    endif ()
  endif ()
else ()
  set (GSPC_NET_FOUND true)
  set (GSPC_NET_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gspcnet;${CMAKE_BINARY_DIR}/gspcnet" CACHE STRING "gspcnet include dir")

  set (GSPC_NET_LIBRARY_STATIC gspcnet CACHE STRING "gspcnet static library")
  set (GSPC_NET_LIBRARY_SHARED gspcnet.shared CACHE STRING "gspcnet shared library")
  set (GSPC_NET_LIBRARY "${GSPC_NET_LIBRARY_STATIC}" CACHE STRING "gspcnet default library")

  if (NOT GSPC_NET_FIND_QUIETLY)
    message(STATUS "Found GSPC_NET headers in ${GSPC_NET_INCLUDE_DIR}")
    if (GSPC_NET_LIBRARY_STATIC)
      message(STATUS "Found GSPC_NET static library in ${GSPC_NET_LIBRARY}")
    endif ()
    if (GSPC_NET_LIBRARY_SHARED)
      message(STATUS "Found GSPC_NET shared library in ${GSPC_NET_LIBRARY_SHARED}")
    endif ()
  endif ()
endif ()

mark_as_advanced (FORCE
  GSPC_NET_LIBRARY
  GSPC_NET_LIBRARY_STATIC
  GSPC_NET_LIBRARY_SHARED
  GSPC_NET_INCLUDE_DIR
)
