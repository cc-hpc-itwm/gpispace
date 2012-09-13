# -*- mode: cmake; -*-
# locates the libzmq library
# This file defines:
# * ZMQ_FOUND if libreadline was found
# * ZMQ_LIBRARY The lib to link to (static)
# * ZMQ_STATIC_LIBRARY
# * ZMQ_SHARED_LIBRARY The lib to link to (shared)
# * ZMQ_INCLUDE_DIR

if (NOT ZMQ_FIND_QUIETLY)
  message(STATUS "FindZMQ check")
endif ()

find_path (ZMQ_INCLUDE_DIR
  NAMES "zmq.h"
  HINTS ${ZMQ_HOME} ENV ZMQ_HOME
  PATH_SUFFIXES include
  )

find_library (ZMQ_STATIC_LIBRARY
  NAMES libzmq.a
  HINTS ${ZMQ_HOME} ENV ZMQ_HOME
  PATH_SUFFIXES lib lib64
  )
find_library (ZMQ_SHARED_LIBRARY
  NAMES libzmq.so
  HINTS ${ZMQ_HOME} ENV ZMQ_HOME
  PATH_SUFFIXES lib lib64
  )

if (ZMQ_STATIC_LIBRARY)
  set(ZMQ_LIBRARY ${ZMQ_STATIC_LIBRARY})
else()
  if (ZMQ_SHARED_LIBRARY)
    set(ZMQ_LIBRARY ${ZMQ_SHARED_LIBRARY})
  endif()
endif()

if (ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY)
  set (ZMQ_FOUND TRUE)
  if (NOT ZMQ_FIND_QUIETLY)
    message (STATUS "Found ZMQ headers in ${ZMQ_INCLUDE_DIR} and libraries ${ZMQ_LIBRARY} ${ZMQ_SHARED_LIBRARY}")
  endif ()
else ()
  if (ZMQ_FIND_REQUIRED)
    message (FATAL_ERROR "ZMQ could not be found!")
  endif ()
endif ()
