# -*- mode: cmake; -*-
# Locate Xerces-C include paths and libraries
# Xerces-C can be found at http://xml.apache.org/xerces-c/
# Written by Frederic Heem, frederic.heem _at_ telsey.it

# This module defines
# XERCESC_INCLUDE_DIR, where to find ptlib.h, etc.
# XERCESC_LIBRARIES, the libraries to link against to use pwlib.
# XERCESC_FOUND, If false, don't try to use pwlib.

IF (NOT XERCESC_FIND_QUIETLY)
  MESSAGE(STATUS "Looking for xerces-c...")
ENDIF (NOT XERCESC_FIND_QUIETLY)

SET( XERCESC_FOUND "NO" )

FIND_PATH(XERCESC_INCLUDE_DIRS xercesc/dom/DOM.hpp
  "[HKEY_CURRENT_USER\\software\\xerces-c\\src]"
  "[HKEY_CURRENT_USER\\xerces-c\\src]"
  $ENV{XERCESCROOT}/src/
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(XERCESC_LIBRARIES
  NAMES 
    xerces-c
  PATHS
    "[HKEY_CURRENT_USER\\software\\xerces-c\\lib]"
    "[HKEY_CURRENT_USER\\xerces-c\\lib]"
    $ENV{XERCESCROOT}/lib
    /usr/local/lib
    /usr/lib
    /usr/lib64
)

# if the include a the library are found then we have it
IF(XERCESC_INCLUDE_DIRS)
  IF(XERCESC_LIBRARIES)
    SET( XERCESC_FOUND "YES" )
    IF (NOT XERCESC_FIND_QUIETLY)
      MESSAGE(STATUS "Looking for xerces-c... - found ${XERCESC_LIBRARIES}")
    ENDIF (NOT XERCESC_FIND_QUIETLY)
  ENDIF(XERCESC_LIBRARIES)
ENDIF(XERCESC_INCLUDE_DIRS)

MARK_AS_ADVANCED(
  XERCESC_INCLUDE_DIRS
  XERCESC_LIBRARIES
  XERCESC_FOUND
)
