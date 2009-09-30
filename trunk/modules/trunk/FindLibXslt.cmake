# -*- mode: cmake; -*-
# Locate libxslt include paths and libraries
# libxslt can be found at http://xmlsoft.org/XSLT/ 

# This module defines
# LIBXSLT_INCLUDE_DIR, where to find header files, etc.
# LIBXSLT_LIBRARIES, the libraries to link against to use libxslt.
# LIBXSLT_FOUND, If false, don't try to use libxslt.

IF (NOT LIBXSLT_FIND_QUIETLY)
  MESSAGE(STATUS "Looking for libxslt...")
ENDIF (NOT LIBXSLT_FIND_QUIETLY)

SET( LIBXSLT_FOUND "NO" )

FIND_PATH(LIBXSLT_INCLUDE_DIRS libxslt/xslt.h
  "[HKEY_CURRENT_USER\\software\\libxslt\\src]"
  "[HKEY_CURRENT_USER\\libxslt\\src]"
  $ENV{LIBXSLTROOT}/src/
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(LIBXSLT_LIBRARIES
  NAMES 
    xslt
  PATHS
    "[HKEY_CURRENT_USER\\software\\libxslt\\lib]"
    "[HKEY_CURRENT_USER\\libxslt\\lib]"
    $ENV{LIBXSLTROOT}/lib
    /usr/local/lib
    /usr/lib
    /usr/lib64
)

# if the include a the library are found then we have it
IF(LIBXSLT_INCLUDE_DIRS)
  MESSAGE(STATUS "Looking for libxslt... - found header files in ${LIBXSLT_INCLUDE_DIRS}")
  IF(LIBXSLT_LIBRARIES)
    SET( LIBXSLT_FOUND "YES" )
    IF (NOT LIBXSLT_FIND_QUIETLY)
      MESSAGE(STATUS "Looking for libxslt... - found library ${LIBXSLT_LIBRARIES}")
    ENDIF (NOT LIBXSLT_FIND_QUIETLY)
  ENDIF(LIBXSLT_LIBRARIES)
ENDIF(LIBXSLT_INCLUDE_DIRS)

MARK_AS_ADVANCED(
  LIBXSLT_INCLUDE_DIRS
  LIBXSLT_LIBRARIES
  LIBXSLT_FOUND
)
