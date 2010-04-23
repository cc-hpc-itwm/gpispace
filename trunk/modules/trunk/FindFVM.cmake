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

message(STATUS "FindFVM check")
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(FindPackageHelper)
  check_package(FVM fvm-pc/pc.hpp fvm 1.0)
else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(FVM_FOUND true)
  set(FVM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/fvm-pc;${CMAKE_BINARY_DIR}/fvm-pc")
  set(FVM_LIBRARY_DIR "${CMAKE_BINARY_DIR}/fvm-pc")
  set(FVM_LIBRARY "${CMAKE_BINARY_DIR}/fvm-pc/libfvm-pc_fake.so")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

