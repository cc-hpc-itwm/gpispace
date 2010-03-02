# -*- mode: cmake; -*-
# Locate libxml2 include paths and libraries
# libxml2 can be found at http://xmlsoft.org/ 

# This module defines
# LIBXML2_INCLUDE_DIR, where to find header files, etc.
# LIBXML2_LIBRARIES, the libraries to link against to use libxml2.
# LIBXML2_FOUND, If false, don't try to use libxml2.

IF (NOT LIBXML2_FIND_QUIETLY)
  MESSAGE(STATUS "Looking for libxml2...")
ENDIF (NOT LIBXML2_FIND_QUIETLY)

SET( LIBXML2_FOUND "NO" )

FIND_PATH(LIBXML2_INCLUDE_DIRS libxml/xpath.h
  "[HKEY_CURRENT_USER\\software\\libxml2\\src]"
  "[HKEY_CURRENT_USER\\libxml2\\src]"
  $ENV{LIBXML2ROOT}/src/
  /usr/local/include
  /usr/local/include/libxml2
  /usr/include
  /usr/include/libxml2
)

FIND_LIBRARY(LIBXML2_LIBRARIES
  NAMES 
    xml2
  PATHS
    "[HKEY_CURRENT_USER\\software\\libxml2\\lib]"
    "[HKEY_CURRENT_USER\\libxml2\\lib]"
    $ENV{LIBXML2ROOT}/lib
    /usr/local/lib
    /usr/lib
    /usr/lib64
)

# if the include a the library are found then we have it
IF(LIBXML2_INCLUDE_DIRS)
  MESSAGE(STATUS "Looking for libxml2... - found header files in ${LIBXML2_INCLUDE_DIRS}")
  IF(LIBXML2_LIBRARIES)
    SET( LIBXML2_FOUND "YES" )
    IF (NOT LIBXML2_FIND_QUIETLY)
      MESSAGE(STATUS "Looking for libxml2... - found library ${LIBXML2_LIBRARIES}")
    ENDIF (NOT LIBXML2_FIND_QUIETLY)
  ENDIF(LIBXML2_LIBRARIES)
ENDIF(LIBXML2_INCLUDE_DIRS)

MARK_AS_ADVANCED(
  LIBXML2_INCLUDE_DIRS
  LIBXML2_LIBRARIES
  LIBXML2_FOUND
)
