# -*- mode: cmake; -*-
# locates the Gwes Workflow Engine
# This file defines:
# * GWES_FOUND if gwes was was found
# * GWES_LIBRARY The lib to link to (currently only a static unix lib) 
# * GWES_INCLUDE_DIR
#
# * GWDL_FOUND if gwdl was was found
# * GWDL_LIBRARY The lib to link to (currently only a static unix lib) 
# * GWDL_INCLUDE_DIR

# set defaults
SET(_gwes_HOME "/usr/local")
SET(_gwes_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_gwes_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  )

SET(_gwes_BINARY_SEARCH_DIRS
  ${CMAKE_BINARY_PATH}
  /usr/local/lib
  /usr/lib
  )

if( "${GWES_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{GWES_HOME}")
    message(STATUS "GWES_HOME env is not set, setting it to ${_gwes_HOME}")
    set (GWES_HOME ${_gwes_HOME})
  else("" MATCHES "$ENV{GWES_HOME}")
    set (GWES_HOME "$ENV{GWES_HOME}")
  endif("" MATCHES "$ENV{GWES_HOME}")
else( "${GWES_HOME}" STREQUAL "")
  message(STATUS "GWES_HOME is not empty: \"${GWES_HOME}\"")
endif( "${GWES_HOME}" STREQUAL "")

message(STATUS "Looking for GWES in ${GWES_HOME}")

# fix searchpath now
IF( NOT ${GWES_HOME} STREQUAL "" )
    SET(_gwes_INCLUDE_SEARCH_DIRS ${GWES_HOME}/include ${_gwes_INCLUDE_SEARCH_DIRS})
    SET(_gwes_LIBRARIES_SEARCH_DIRS ${GWES_HOME}/lib ${_gwes_LIBRARIES_SEARCH_DIRS})
    SET(_gwes_HOME ${GWES_HOME})
ENDIF( NOT ${GWES_HOME} STREQUAL "" )

IF( NOT $ENV{GWES_INCLUDEDIR} STREQUAL "" )
  SET(_gwes_INCLUDE_SEARCH_DIRS $ENV{GWES_INCLUDEDIR} ${_gwes_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{GWES_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{GWES_LIBRARYDIR} STREQUAL "" )
  SET(_gwes_LIBRARIES_SEARCH_DIRS $ENV{GWES_LIBRARYDIR} ${_gwes_LIBRARIES_SEARCH_DIRS})
ENDIF( NOT $ENV{GWES_LIBRARYDIR} STREQUAL "" )

IF( GWES_HOME )
  SET(_gwes_INCLUDE_SEARCH_DIRS ${GWES_HOME}/include ${_gwes_INCLUDE_SEARCH_DIRS})
  SET(_gwes_LIBRARIES_SEARCH_DIRS ${GWES_HOME}/lib ${_gwes_LIBRARIES_SEARCH_DIRS})
  SET(_gwes_HOME ${GWES_HOME})
ENDIF( GWES_HOME )

# find the include files
FIND_PATH(GWES_INCLUDE_DIR
  NAMES gwes/GWES.h
  HINTS ${_gwes_INCLUDE_SEARCH_DIRS}
)

IF(WIN32)
  SET(GWES_LIBRARY_NAMES ${GWES_LIBRARY_NAMES} libgwes_cpp.lib)
ELSE(WIN32)
  SET(GWES_LIBRARY_NAMES ${GWES_LIBRARY_NAMES} libgwes_cpp.a libgwes_cpp.so)
ENDIF(WIN32)

FIND_LIBRARY(GWES_LIBRARY
  NAMES ${GWES_LIBRARY_NAMES}
  HINTS ${_gwes_LIBRARIES_SEARCH_DIRS}
  )

# if the include and the program are found then we have it
IF(GWES_INCLUDE_DIR AND GWES_LIBRARY) 
  SET(GWES_FOUND "YES")
  message(STATUS "Looking for GWES ... found headers in ${GWES_INCLUDE_DIR} and library ${GWES_LIBRARY}")
else(GWES_INCLUDE_DIR AND GWES_LIBRARY) 
  message(STATUS "GWES could not be found, try setting GWES_HOME (value=\"${GWES_HOME}\").")
ENDIF(GWES_INCLUDE_DIR AND GWES_LIBRARY)

MARK_AS_ADVANCED(
  GWES_FOUND
  GWES_HOME
  GWES_LIBRARY
  GWES_INCLUDE_DIR
)

##
##
## GWDL stuff
##
##

# set defaults
SET(_gwdl_HOME "/usr/local")
SET(_gwdl_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_gwdl_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  )

SET(_gwdl_BINARY_SEARCH_DIRS
  ${CMAKE_BINARY_PATH}
  /usr/local/lib
  /usr/lib
  )

##
if( "${GWDL_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{GWDL_HOME}")
    message(STATUS "GWDL_HOME env is not set, setting it to ${_gwdl_HOME}")
    set (GWDL_HOME ${_gwdl_HOME})
  else("" MATCHES "$ENV{GWDL_HOME}")
    set (GWDL_HOME "$ENV{GWDL_HOME}")
  endif("" MATCHES "$ENV{GWDL_HOME}")
else( "${GWDL_HOME}" STREQUAL "")
  message(STATUS "GWDL_HOME is not empty: \"${GWDL_HOME}\"")
endif( "${GWDL_HOME}" STREQUAL "")
##

message(STATUS "Looking for GWDL in ${GWDL_HOME}")

# fix searchpath now
IF( NOT ${GWDL_HOME} STREQUAL "" )
    SET(_gwdl_INCLUDE_SEARCH_DIRS ${GWDL_HOME}/include ${_gwdl_INCLUDE_SEARCH_DIRS})
    SET(_gwdl_LIBRARIES_SEARCH_DIRS ${GWDL_HOME}/lib ${_gwdl_LIBRARIES_SEARCH_DIRS})
    SET(_gwdl_HOME ${GWDL_HOME})
ENDIF( NOT ${GWDL_HOME} STREQUAL "" )

IF( NOT $ENV{GWDL_INCLUDEDIR} STREQUAL "" )
  SET(_gwdl_INCLUDE_SEARCH_DIRS $ENV{GWDL_INCLUDEDIR} ${_gwdl_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{GWDL_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{GWDL_LIBRARYDIR} STREQUAL "" )
  SET(_gwdl_LIBRARIES_SEARCH_DIRS $ENV{GWDL_LIBRARYDIR} ${_gwdl_LIBRARIES_SEARCH_DIRS})
ENDIF( NOT $ENV{GWDL_LIBRARYDIR} STREQUAL "" )

IF( GWDL_HOME )
  SET(_gwdl_INCLUDE_SEARCH_DIRS ${GWDL_HOME}/include ${_gwdl_INCLUDE_SEARCH_DIRS})
  SET(_gwdl_LIBRARIES_SEARCH_DIRS ${GWDL_HOME}/lib ${_gwdl_LIBRARIES_SEARCH_DIRS})
  SET(_gwdl_HOME ${GWDL_HOME})
ENDIF( GWDL_HOME )

# find the include files
FIND_PATH(GWDL_INCLUDE_DIR
  NAMES gwdl/Workflow.h
  HINTS ${_gwdl_INCLUDE_SEARCH_DIRS}
)

IF(WIN32)
  SET(GWDL_LIBRARY_NAMES ${GWDL_LIBRARY_NAMES} libgworkflowdl_cpp.lib)
ELSE(WIN32)
  SET(GWDL_LIBRARY_NAMES ${GWDL_LIBRARY_NAMES} libgworkflowdl_cpp.a libgworkflowdl_cpp.so)
ENDIF(WIN32)

FIND_LIBRARY(GWDL_LIBRARY
  NAMES ${GWDL_LIBRARY_NAMES}
  HINTS ${_gwdl_LIBRARIES_SEARCH_DIRS}
  )

# if the include and the program are found then we have it
IF(GWDL_INCLUDE_DIR AND GWDL_LIBRARY) 
  SET(GWDL_FOUND "YES")
  message(STATUS "Looking for GWDL ... found headers in ${GWDL_INCLUDE_DIR} and library ${GWDL_LIBRARY}")
else(GWDL_INCLUDE_DIR AND GWDL_LIBRARY) 
  message(STATUS "GWDL could not be found, try setting GWDL_HOME (value=\"${GWDL_HOME}\").")
ENDIF(GWDL_INCLUDE_DIR AND GWDL_LIBRARY)

MARK_AS_ADVANCED(
  GWDL_FOUND
  GWDL_HOME
  GWDL_LIBRARY
  GWDL_INCLUDE_DIR
)

