# -*- mode: cmake; -*-
# locates the FHG Logging framework
# This file defines:
# * FHGLOG_FOUND if fhglog was found
# * FHGLOG_LIBRARY The lib to link to (currently only a static unix lib) 
# * FHGLOG_INCLUDE_DIR

message(STATUS "FindFhglog check")

IF (NOT WIN32)
  include(FindPkgConfig)
  if ( PKG_CONFIG_FOUND )

     pkg_check_modules (PC_FHGLOG fhglog>=1.0)

  endif(PKG_CONFIG_FOUND)
endif (NOT WIN32)

# set defaults
SET(_fhglog_HOME "/usr/local")
SET(_fhglog_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_fhglog_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  )

SET(_fhglog_BINARY_SEARCH_DIRS
  ${CMAKE_BINARY_PATH}
  /usr/local/lib
  /usr/lib
  )

##
if( "${FHGLOG_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{FHGLOG_HOME}")
    message(STATUS "FHGLOG_HOME env is not set, setting it to /usr/local")
    set (FHGLOG_HOME ${_fhglog_HOME})
  else("" MATCHES "$ENV{FHGLOG_HOME}")
    set (FHGLOG_HOME "$ENV{FHGLOG_HOME}")
  endif("" MATCHES "$ENV{FHGLOG_HOME}")
else( "${FHGLOG_HOME}" STREQUAL "")
  message(STATUS "FHGLOG_HOME is not empty: \"${FHGLOG_HOME}\"")
endif( "${FHGLOG_HOME}" STREQUAL "")
##

message(STATUS "Looking for fhglog in ${FHGLOG_HOME}")

IF( NOT ${FHGLOG_HOME} STREQUAL "" )
    SET(_fhglog_INCLUDE_SEARCH_DIRS ${FHGLOG_HOME}/include ${_fhglog_INCLUDE_SEARCH_DIRS})
    SET(_fhglog_LIBRARIES_SEARCH_DIRS ${FHGLOG_HOME}/lib ${_fhglog_LIBRARIES_SEARCH_DIRS})
    SET(_fhglog_BINARY_SEARCH_DIRS ${FHGLOG_HOME}/bin ${_fhglog_BINARY_SEARCH_DIRS})
    SET(_fhglog_HOME ${FHGLOG_HOME})
ENDIF( NOT ${FHGLOG_HOME} STREQUAL "" )

IF( NOT $ENV{FHGLOG_INCLUDEDIR} STREQUAL "" )
  SET(_fhglog_INCLUDE_SEARCH_DIRS $ENV{FHGLOG_INCLUDEDIR} ${_fhglog_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{FHGLOG_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{FHGLOG_LIBRARYDIR} STREQUAL "" )
  SET(_fhglog_LIBRARIES_SEARCH_DIRS $ENV{FHGLOG_LIBRARYDIR} ${_fhglog_LIBRARIES_SEARCH_DIRS})
ENDIF( NOT $ENV{FHGLOG_LIBRARYDIR} STREQUAL "" )

IF( NOT $ENV{FHGLOG_BINARYDIR} STREQUAL "" )
  SET(_fhglog_BINARY_SEARCH_DIRS $ENV{FHGLOG_BINARYDIR} ${_fhglog_BINARY_SEARCH_DIRS})
ENDIF( NOT $ENV{FHGLOG_BINARYDIR} STREQUAL "" )

IF( FHGLOG_HOME )
  SET(_fhglog_INCLUDE_SEARCH_DIRS ${FHGLOG_HOME}/include ${_fhglog_INCLUDE_SEARCH_DIRS})
  SET(_fhglog_LIBRARIES_SEARCH_DIRS ${FHGLOG_HOME}/lib ${_fhglog_LIBRARIES_SEARCH_DIRS})
  SET(_fhglog_BINARY_SEARCH_DIRS ${FHGLOG_HOME}/lib ${_fhglog_BINARY_SEARCH_DIRS})
  SET(_fhglog_HOME ${FHGLOG_HOME})
ENDIF( FHGLOG_HOME )

# find the include files
FIND_PATH(FHGLOG_INCLUDE_DIR 
  NAMES fhglog/Logger.hpp
  HINTS ${_fhglog_INCLUDE_SEARCH_DIRS}
)

IF(WIN32)
  SET(FHGLOG_LIBRARY_NAMES ${FHGLOG_LIBRARY_NAMES} libfhglog.lib)
ELSE(WIN32)
  SET(FHGLOG_LIBRARY_NAMES ${FHGLOG_LIBRARY_NAMES} libfhglog.a libfhglog.so)
ENDIF(WIN32)

FIND_LIBRARY(FHGLOG_LIBRARY
  NAMES ${FHGLOG_LIBRARY_NAMES}
  HINTS ${_fhglog_LIBRARIES_SEARCH_DIRS}
  )

# if the include and the program are found then we have it
IF(FHGLOG_INCLUDE_DIR AND FHGLOG_LIBRARY) 
  SET(FHGLOG_FOUND "YES")
  message(STATUS "Looking for FHGLog ... found headers in ${FHGLOG_INCLUDE_DIR} and library ${FHGLOG_LIBRARY}")
else(FHGLOG_INCLUDE_DIR AND FHGLOG_LIBRARY) 
  message(STATUS "FHGLog could not be found, try setting FHGLOG_HOME (value=\"${FHGLOG_HOME}\").")
ENDIF(FHGLOG_INCLUDE_DIR AND FHGLOG_LIBRARY)

MARK_AS_ADVANCED(
  FHGLOG_FOUND
  FHGLOG_HOME
  FHGLOG_LIBRARY
  FHGLOG_INCLUDE_DIR
)
