# -*- mode: cmake; -*-
# locates the libczmq library
# This file defines:
# * CZMQ_FOUND if libreadline was found
# * CZMQ_LIBRARY The lib to link to (static)
# * CZMQ_STATIC_LIBRARY
# * CZMQ_SHARED_LIBRARY The lib to link to (shared)
# * CZMQ_INCLUDE_DIR

if (NOT CZMQ_FIND_QUIETLY)
  message(STATUS "FindCZMQ check")
endif ()

find_path (CZMQ_INCLUDE_DIR
  NAMES "czmq.h"
  HINTS ${CZMQ_HOME} ENV CZMQ_HOME
  PATH_SUFFIXES include
  )

find_library (CZMQ_STATIC_LIBRARY
  NAMES libczmq.a
  HINTS ${CZMQ_HOME} ENV CZMQ_HOME
  PATH_SUFFIXES lib lib64
  )
find_library (CZMQ_SHARED_LIBRARY
  NAMES libczmq.so
  HINTS ${CZMQ_HOME} ENV CZMQ_HOME
  PATH_SUFFIXES lib lib64
  )

if (CZMQ_STATIC_LIBRARY)
  set(CZMQ_LIBRARY ${CZMQ_STATIC_LIBRARY})
else()
  if (CZMQ_SHARED_LIBRARY)
    set(CZMQ_LIBRARY ${CZMQ_SHARED_LIBRARY})
  endif()
endif()

if (CZMQ_INCLUDE_DIR AND CZMQ_LIBRARY)
  set (CZMQ_FOUND TRUE)
  if (NOT CZMQ_FIND_QUIETLY)
    message (STATUS "Found CZMQ headers in ${CZMQ_INCLUDE_DIR} and libraries ${CZMQ_LIBRARY} ${CZMQ_SHARED_LIBRARY}")
  endif ()
else ()
  if (CZMQ_FIND_REQUIRED)
    message (FATAL_ERROR "CZMQ could not be found!")
  endif ()
endif ()
