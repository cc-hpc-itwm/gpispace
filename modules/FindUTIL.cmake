# -*- mode: cmake; -*-
# This file defines:
# * UTIL_FOUND if workflow engine was found
# * UTIL_INCLUDE_DIR The path to the include directory
# *  PKG_CHECK_MODULE used to set the following variables
# *
# *   <PREFIX>_FOUND           ... set to 1 if module(s) exist
# *   <XPREFIX>_LIBRARIES      ... only the libraries (w/o the '-l')
# *   <XPREFIX>_LIBRARY_DIRS   ... the paths of the libraries (w/o the '-L')
# *   <XPREFIX>_LDFLAGS        ... all required linker flags
# *   <XPREFIX>_LDFLAGS_OTHER  ... all other linker flags
# *   <XPREFIX>_INCLUDE_DIRS   ... the '-I' preprocessor flags (w/o the '-I')
# *   <XPREFIX>_CFLAGS         ... all required cflags
# *   <XPREFIX>_CFLAGS_OTHER   ... the other compiler flags

# *   <XPREFIX> = <PREFIX>_STATIC for static linking
# *   <XPREFIX>_VERSION    ... version of the module
# *   <XPREFIX>_PREFIX     ... prefix-directory of the module
# *   <XPREFIX>_INCLUDEDIR ... include-dir of the module
# *   <XPREFIX>_LIBDIR     ... lib-dir of the module
# *   <XPREFIX> = <PREFIX>  when |MODULES| == 1, else
# *   <XPREFIX> = <PREFIX>_<MODNAME>

if (NOT UTIL_FIND_QUIETLY)
  message(STATUS "FindUTIL check")
endif (NOT UTIL_FIND_QUIETLY)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  find_path (UTIL_INCLUDE_DIR
    NAMES "fhg/util/util.hpp"
    HINTS ${UTIL_HOME} ENV UTIL_HOME
    PATH_SUFFIXES include
    )

  if (UTIL_INCLUDE_DIR)
    set (UTIL_FOUND TRUE)
    if (NOT UTIL_FIND_QUIETLY)
      message(STATUS "Found UTIL headers in ${UTIL_INCLUDE_DIR}")
    endif (NOT UTIL_FIND_QUIETLY)
  else (UTIL_INCLUDE_DIR)
    if (UTIL_FIND_REQUIRED)
      message (FATAL_ERROR "UTIL could not be found!")
    endif (UTIL_FIND_REQUIRED)
  endif (UTIL_INCLUDE_DIR)

  # TODO: Find fhg-revision!

else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(UTIL_FOUND true)
  set(UTIL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/util;${CMAKE_BINARY_DIR}/util")
  set(UTIL_LIBRARIES fhg-revision)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
