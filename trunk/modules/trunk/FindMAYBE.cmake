# -*- mode: cmake; -*-
# This file defines:
# * MAYBE_FOUND if workflow engine was found
# * MAYBE_INCLUDE_DIR The path to the include directory
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

if (NOT MAYBE_FIND_QUIETLY)
  message(STATUS "FindMAYBE check")
endif (NOT MAYBE_FIND_QUIETLY)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  find_path (MAYBE_INCLUDE_DIR
    NAMES "xml/parse/util/maybe.hpp"
    HINTS ${MAYBE_HOME} ENV MAYBE_HOME
    PATH_SUFFIXES include
    )

  if (MAYBE_INCLUDE_DIR)
    set (MAYBE_FOUND TRUE)
    if (NOT MAYBE_FIND_QUIETLY)
      message(STATUS "Found MAYBE headers in ${MAYBE_INCLUDE_DIR}")
      if (MAYBE_LOADER)
	message(STATUS "Found MAYBE loader tool in ${MAYBE_LOADER_BIN}")
      endif (MAYBE_LOADER)
    endif (NOT MAYBE_FIND_QUIETLY)
  else (MAYBE_INCLUDE_DIR)
    if (MAYBE_FIND_REQUIRED)
      message (FATAL_ERROR "MAYBE could not be found!")
    endif (MAYBE_FIND_REQUIRED)
  endif (MAYBE_INCLUDE_DIR)
else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(MAYBE_FOUND true)
  set(MAYBE_INCLUDE_DIR "${CMAKE_SOURCE_DIR};${CMAKE_BINARY_DIR}")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
