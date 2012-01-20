# -*- mode: cmake; -*-
# This file defines:
# * MMGR_FOUND if workflow engine was found
# * MMGR_LIBRARY The lib to link to (currently only a static unix lib, not portable)
# * MMGR_INCLUDE_DIR The path to the include directory
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

if (NOT MMGR_FIND_QUIETLY)
  message(STATUS "FindMMGR check")
endif (NOT MMGR_FIND_QUIETLY)

if(NOT TARGET mmgr)
  find_path (MMGR_INCLUDE_DIR
	NAMES "mmgr/dtmmgr.h"
	HINTS ${MMGR_HOME} ENV MMGR_HOME
	PATH_SUFFIXES include
  )

  find_library (MMGR_LIBRARY
	NAMES libmmgr.a
	HINTS ${MMGR_HOME} ENV MMGR_HOME
	PATH_SUFFIXES lib
  )
  find_library (MMGR_LIBRARY_SHARED
	NAMES libmmgr${CMAKE_FIND_LIBRARY_SUFFIXES}
	HINTS ${MMGR_HOME} ENV MMGR_HOME
	PATH_SUFFIXES lib
  )

  if (MMGR_INCLUDE_DIR)
    set (MMGR_FOUND TRUE)
  else (MMGR_INCLUDE_DIR)
    if (MMGR_FIND_REQUIRED)
      message (FATAL_ERROR "MMGR could not be found!")
    endif (MMGR_FIND_REQUIRED)
  endif (MMGR_INCLUDE_DIR)
else()
  set(MMGR_FOUND true)
  set(MMGR_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/mmgr;${CMAKE_BINARY_DIR}/mmgr")
  set(MMGR_LIBRARY mmgr)
  set(MMGR_LIBRARY_SHARED mmgr.shared)
endif()

if (NOT MMGR_FIND_QUIETLY)
  if (MMGR_FOUND)
    message (STATUS "Found MMGR headers in: ${MMGR_INCLUDE_DIR}")
    if (MMGR_LIBRARY)
      message (STATUS "Found MMGR static library: ${MMGR_LIBRARY}")
    endif()
    if (MMGR_LIBRARY_SHARED)
      message (STATUS "Found MMGR shared library: ${MMGR_LIBRARY_SHARED}")
    endif()
  endif()
endif()
