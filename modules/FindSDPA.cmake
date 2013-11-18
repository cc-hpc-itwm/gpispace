# -*- mode: cmake; -*-
# locates the SDPA framework
# This file defines:
# * SDPA_FOUND if fhglog was found
# * SDPA_LIBRARY The lib to link to (currently only a static unix lib)
# * SDPA_INCLUDE_DIR

if (NOT SDPA_FIND_QUIETLY)
  message(STATUS "FindSDPA check")
endif (NOT SDPA_FIND_QUIETLY)

if (NOT TARGET sdpac)
  find_path (SDPA_INCLUDE_DIR
        NAMES "sdpa/daemon/GenericDaemon.hpp"
        HINTS ${SDPA_HOME} ENV SDPA_HOME
        PATH_SUFFIXES include
  )

  find_library (SDPA_LIBRARY
        NAMES libsdpa-daemon.a
        HINTS ${SDPA_HOME} ENV SDPA_HOME
        PATH_SUFFIXES lib
  )

  if (SDPA_INCLUDE_DIR AND SDPA_LIBRARY)
        set (SDPA_FOUND TRUE)
        if (NOT SDPA_FIND_QUIETLY)
          message (STATUS "Found SDPA headers in ${SDPA_INCLUDE_DIR} and libraries ${SDPA_LIBRARY} ${SDPA_LIBRARY_SHARED}")
        endif (NOT SDPA_FIND_QUIETLY)
  else (SDPA_INCLUDE_DIR AND SDPA_LIBRARY)
        if (SDPA_FIND_REQUIRED)
          message (FATAL_ERROR "SDPA could not be found!")
        endif (SDPA_FIND_REQUIRED)
  endif (SDPA_INCLUDE_DIR AND SDPA_LIBRARY)

else()
  set(SDPA_FOUND true)
  set(SDPA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/sdpa ${CMAKE_BINARY_DIR}/sdpa)
  set(SDPA_LIBRARY_DIR "")
  set(SDPA_LIBRARY sdpa-daemon)
endif()
