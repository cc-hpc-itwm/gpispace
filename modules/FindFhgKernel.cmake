# -*- mode: cmake; -*-
# This file defines:
# * FhgKernel_FOUND if workflow engine was found
# * FhgKernel_INCLUDE_DIR The path to the include directory
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

if (NOT FhgKernel_FIND_QUIETLY)
  message(STATUS "FindFhgKernel check")
endif (NOT FhgKernel_FIND_QUIETLY)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  find_path (FhgKernel_INCLUDE_DIR
    NAMES "fhg/plugin/kernel.hpp"
    HINTS ${FhgKernel_HOME} ENV FhgKernel_HOME
    PATH_SUFFIXES include
    )

  if (FhgKernel_INCLUDE_DIR)
    set (FhgKernel_FOUND TRUE)
    if (NOT FhgKernel_FIND_QUIETLY)
      message(STATUS "Found FhgKernel headers in ${FhgKernel_INCLUDE_DIR}")
    endif (NOT FhgKernel_FIND_QUIETLY)
  else (FhgKernel_INCLUDE_DIR)
    if (FhgKernel_FIND_REQUIRED)
      message (FATAL_ERROR "FhgKernel could not be found!")
    endif (FhgKernel_FIND_REQUIRED)
  endif (FhgKernel_INCLUDE_DIR)
else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(FhgKernel_FOUND true)
  set(FhgKernel_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/fhgkernel;${CMAKE_BINARY_DIR}/fhgkernel")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
