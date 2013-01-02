# -*- mode: cmake; -*-
# This file defines:
# * WE_FOUND if workflow engine was found
# * WE_LIBRARY The lib to link to (currently only a static unix lib, not portable)
# * WE_INCLUDE_DIR The path to the include directory
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

if (NOT WE_FIND_QUIETLY)
  message(STATUS "FindWE check")
endif (NOT WE_FIND_QUIETLY)

if(NOT TARGET we-exec)
# if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  find_path (WE_INCLUDE_DIR
    NAMES "we/type/net.hpp"
    HINTS ${WE_HOME} ENV WE_HOME
    PATH_SUFFIXES include
    )

  find_program (WE_LOADER
    NAMES "loader"
    HINTS ${WE_HOME} ENV WE_HOME
    PATH_SUFFIXES bin
    )

  if (WE_INCLUDE_DIR)
    set (WE_FOUND TRUE)
    if (NOT WE_FIND_QUIETLY)
      message(STATUS "Found WE headers in ${WE_INCLUDE_DIR}")
      if (WE_LOADER)
	message(STATUS "Found WE loader tool in ${WE_LOADER_BIN}")
      endif (WE_LOADER)
    endif (NOT WE_FIND_QUIETLY)
  else (WE_INCLUDE_DIR)
    if (WE_FIND_REQUIRED)
      message (FATAL_ERROR "WE could not be found!")
    endif (WE_FIND_REQUIRED)
  endif (WE_INCLUDE_DIR)
  #! \todo find we_type
else(NOT TARGET we-exec)
# else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(WE_FOUND true)
  set(WE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/we;${CMAKE_BINARY_DIR}/we")
  set(WE_INCLUDE_SRC_DIR "${CMAKE_SOURCE_DIR}/we")
  set(WE_LIBRARY_DIR "")
  set(WE_LIBRARIES ${WE_LIBRARIES})
endif(NOT TARGET we-exec)
# endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
