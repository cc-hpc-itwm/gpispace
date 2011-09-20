# -*- mode: cmake; -*-
# - Find MNG
# Find the MNG library and include files
# This module defines
#  MNG_INCLUDE_DIR, where to find libmng.h, etc.
#  MNG_LIBRARIES, the libraries needed to use MNG.
#  MNG_FOUND, If false, do not try to use MNG.
# also defined, but not for general use are
#  MNG_LIBRARY, where to find the MNG library.

set( _mng_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
if(WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
else(WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif(WIN32)

FIND_PATH(MNG_INCLUDE_DIR
  NAMES libmng.h
  HINTS ${MNG_HOME} ENV MNG_HOME
  PATHS /usr/local /usr
  PATH_SUFFIXES include
)

SET(MNG_NAMES ${MNG_NAMES} libmng.a mng)
FIND_LIBRARY(MNG_LIBRARY
  NAMES ${MNG_NAMES}
  HINTS ${MNG_HOME} ENV MNG_HOME
  PATHS /usr /usr/local
  PATH_SUFFIXES lib lib64
)

SET(LCMS_NAMES ${LCMS_NAMES} liblcms.a lcms)
FIND_LIBRARY(LCMS_LIBRARY
  NAMES ${LCMS_NAMES}
  HINTS ${LCMS_HOME} ENV LCMS_HOME
  PATHS /usr /usr/local
  PATH_SUFFIXES lib lib64
)

# handle the QUIETLY and REQUIRED arguments and set JPEG_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MNG DEFAULT_MSG MNG_LIBRARY MNG_INCLUDE_DIR)

MARK_AS_ADVANCED(
  MNG_LIBRARY
  MNG_INCLUDE_DIR
)
