# -*- mode: cmake; -*-
# - Find LCMS
# Find the LCMS library and include files
# This module defines
#  LCMS_INCLUDE_DIR, where to find lcms.h, etc.
#  LCMS_LIBRARIES, the libraries needed to use LCMS.
#  LCMS_FOUND, If false, do not try to use LCMS.
# also defined, but not for general use are
#  LCMS_LIBRARY, where to find the LCMS library.

set( _lcms_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
if(WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
else(WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif(WIN32)

FIND_PATH(LCMS_INCLUDE_DIR
  NAMES lcms.h
  HINTS ${LCMS_HOME} ENV LCMS_HOME
  PATHS /usr/local /usr
  PATH_SUFFIXES include
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
find_package_handle_standard_args(LCMS DEFAULT_MSG LCMS_LIBRARY LCMS_INCLUDE_DIR)

MARK_AS_ADVANCED(
  LCMS_LIBRARY
  LCMS_INCLUDE_DIR
)
