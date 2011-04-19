# -*- mode: cmake; -*-
# This file defines:
# * FVM_FOUND if workflow engine was found
# * FVM_LIBRARY The lib to link to (currently only a static unix lib, not portable)
# * FVM_INCLUDE_DIR The path to the include directory
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

if (NOT FVM_FIND_QUIETLY)
  message(STATUS "FindFVM check")
endif (NOT FVM_FIND_QUIETLY)

# if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT TARGET fvm-pc)
# if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  find_path (FVM_INCLUDE_DIR
    NAMES "fvm-pc/pc.hpp"
    HINTS ${FVM_HOME} ENV FVM_HOME
    PATH_SUFFIXES include
    )

  find_library (FVM_LIBRARY
    NAMES libfvm-pc.a
    HINTS ${FVM_HOME} ENV FVM_HOME
    PATH_SUFFIXES lib
    )
  find_library (FVM_LIBRARY_SHARED
    NAMES libfvm-pc.so
    HINTS ${FVM_HOME} ENV FVM_HOME
    PATH_SUFFIXES lib
    )

  find_library (FVM_FAKE_LIBRARY
    NAMES libfvm-pc.a
    HINTS ${FVM_HOME} ENV FVM_HOME
    PATH_SUFFIXES lib/fake
    )
  find_library (FVM_FAKE_LIBRARY_SHARED
    NAMES libfvm-pc.so
    HINTS ${FVM_HOME} ENV FVM_HOME
    PATH_SUFFIXES lib/fake
    )

  if (FVM_INCLUDE_DIR)
    set (FVM_FOUND TRUE)
    if (NOT FVM_FIND_QUIETLY)
      message(STATUS "Found FVM headers in ${FVM_INCLUDE_DIR}")
      if (FVM_LIBRARY)
	message(STATUS "Found FVM static library in ${FVM_LIBRARY}")
	message(STATUS "Found FVM static fake-library in ${FVM_FAKE_LIBRARY}")
      endif (FVM_LIBRARY)
      if (FVM_LIBRARY_SHARED)
	message(STATUS "Found FVM shared library in ${FVM_LIBRARY_SHARED}")
	message(STATUS "Found FVM shared fake-library in ${FVM_FAKE_LIBRARY_SHARED}")
      endif (FVM_LIBRARY_SHARED)
    endif (NOT FVM_FIND_QUIETLY)
  else (FVM_INCLUDE_DIR)
    if (FVM_FIND_REQUIRED)
      message (FATAL_ERROR "FVM could not be found!")
    endif (FVM_FIND_REQUIRED)
  endif (FVM_INCLUDE_DIR)
# else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
else(NOT TARGET fvm-pc)
  set(FVM_FOUND true)
  set(FVM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/fvm-pc;${CMAKE_BINARY_DIR}/fvm-pc")

  set(FVM_LIBRARY_DIR "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc")
  get_target_property(FVM_LIBRARY fvm-pc.shared LOCATION)
  get_target_property(FVM_STATIC_LIBRARY fvm-pc LOCATION)
  # set(FVM_LIBRARY "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/libfvm-pc.so")
  # set(FVM_STATIC_LIBRARY "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/libfvm-pc.a")

  set(FVM_FAKE_LIBRARY_DIR "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/fake")
  set(FVM_FAKE_LIBRARY "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/fake/libfvm-pc.so")
  set(FVM_FAKE_STATIC_LIBRARY "${CMAKE_BINARY_DIR}/fvm-pc/fvm-pc/fake/libfvm-pc.a")

  if (NOT FVM_FIND_QUIETLY)
    message(STATUS "Found FVM headers in ${FVM_INCLUDE_DIR}")
    if (FVM_LIBRARY)
      message(STATUS "Found FVM static library in ${FVM_LIBRARY}")
      message(STATUS "Found FVM static fake-library in ${FVM_FAKE_LIBRARY}")
    endif (FVM_LIBRARY)
    if (FVM_LIBRARY_SHARED)
      message(STATUS "Found FVM shared library in ${FVM_LIBRARY_SHARED}")
      message(STATUS "Found FVM shared fake-library in ${FVM_FAKE_LIBRARY_SHARED}")
    endif (FVM_LIBRARY_SHARED)
  endif (NOT FVM_FIND_QUIETLY)
endif(NOT TARGET fvm-pc)
# endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

