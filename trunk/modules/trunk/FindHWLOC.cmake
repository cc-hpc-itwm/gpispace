# -*- mode: cmake; -*-
# locates the libhwloc library
# This file defines:
# * HWLOC_FOUND if libreadline was found
# * HWLOC_LIBRARY The lib to link to (currently only a static unix lib)
# * HWLOC_LIBRARY_SHARED The lib to link to (currently only a static unix lib)
# * HWLOC_INCLUDE_DIR

if (NOT HWLOC_FIND_QUIETLY)
  message(STATUS "FindHWLOC check")
endif ()

find_path (HWLOC_INCLUDE_DIR
  NAMES "hwloc/cpuset.h"
  HINTS ${HWLOC_HOME} ENV HWLOC_HOME
  PATH_SUFFIXES include
  )

find_library (HWLOC_LIBRARY
  NAMES hwloc
  HINTS ${HWLOC_HOME} ENV HWLOC_HOME
  PATH_SUFFIXES lib lib64
  )

if (HWLOC_INCLUDE_DIR AND HWLOC_LIBRARY)
  set (HWLOC_FOUND TRUE)
  if (NOT HWLOC_FIND_QUIETLY)
    message (STATUS "Found HWLOC headers in ${HWLOC_INCLUDE_DIR} and libraries ${HWLOC_LIBRARY} ${HWLOC_LIBRARY_SHARED}")
  endif ()
else ()
  if (HWLOC_FIND_REQUIRED)
    message (FATAL_ERROR "HWLOC could not be found!")
  endif ()
endif ()
