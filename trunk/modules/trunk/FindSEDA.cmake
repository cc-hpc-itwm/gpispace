# -*- mode: cmake; -*-
# Tries to locate a logging framework.
# This file defines:
# * SEDA_FOUND if log4cpp was found
# * SEDA_LIBRARY The lib to link to (currently only a static unix lib, not portable) 
# * SEDA_INCLUDE_DIR The path to the include directory
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

message(STATUS "FindSEDA check")
IF (NOT WIN32)
  include(FindPkgConfig)
  if ( PKG_CONFIG_FOUND )

   pkg_check_modules (PC_SEDA libseda>=0.1)

   message(STATUS "SEDA: -I${PC_SEDA_INCLUDE_DIRS} -L${PC_SEDA_LIBRARY_DIRS} -l${PC_SEDA_LIBRARIES} f=${PC_SEDA_FOUND}")
  endif(PKG_CONFIG_FOUND)
endif (NOT WIN32)

SET(_seda_HOME "/usr/local")
SET(_seda_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_seda_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  )

SET(_seda_BINARY_SEARCH_DIRS
  ${CMAKE_BINARY_PATH}
  /usr/local/lib
  /usr/lib
  )

##
if( "${SEDA_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{SEDA_HOME}")
    message(STATUS "SEDA_HOME env is not set, setting it to /usr/local")
    set (SEDA_HOME ${_seda_HOME})
  else("" MATCHES "$ENV{SEDA_HOME}")
    set (SEDA_HOME "$ENV{SEDA_HOME}")
  endif("" MATCHES "$ENV{SEDA_HOME}")
else( "${SEDA_HOME}" STREQUAL "")
  message(STATUS "SEDA_HOME is not empty: \"${SEDA_HOME}\"")
endif( "${SEDA_HOME}" STREQUAL "")
##

message(STATUS "Looking for seda in ${SEDA_HOME}")

IF( NOT ${SEDA_HOME} STREQUAL "" )
    SET(_seda_INCLUDE_SEARCH_DIRS ${SEDA_HOME}/include ${_seda_INCLUDE_SEARCH_DIRS})
    SET(_seda_LIBRARIES_SEARCH_DIRS ${SEDA_HOME}/lib ${_seda_LIBRARIES_SEARCH_DIRS})
    SET(_seda_HOME ${SEDA_HOME})
ENDIF( NOT ${SEDA_HOME} STREQUAL "" )

IF( NOT $ENV{SEDA_INCLUDEDIR} STREQUAL "" )
  SET(_seda_INCLUDE_SEARCH_DIRS $ENV{SEDA_INCLUDEDIR} ${_seda_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{SEDA_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{SEDA_LIBRARYDIR} STREQUAL "" )
  SET(_seda_LIBRARIES_SEARCH_DIRS $ENV{SEDA_LIBRARYDIR} ${_seda_LIBRARIES_SEARCH_DIRS})
ENDIF( NOT $ENV{SEDA_LIBRARYDIR} STREQUAL "" )

IF( SEDA_HOME )
  SET(_seda_INCLUDE_SEARCH_DIRS ${SEDA_HOME}/include ${_seda_INCLUDE_SEARCH_DIRS})
  SET(_seda_LIBRARIES_SEARCH_DIRS ${SEDA_HOME}/lib ${_seda_LIBRARIES_SEARCH_DIRS})
  SET(_seda_HOME ${SEDA_HOME})
ENDIF( SEDA_HOME )

#############################

FIND_PATH(SEDA_INCLUDE_DIRS seda/IEvent.hpp
   HINTS
     ${PC_SEDA_INCLUDEDIR}
     ${PC_SEDA_INCLUDE_DIRS}
     ${_seda_INCLUDE_SEARCH_DIRS}
     ${CMAKE_INCLUDE_PATH}
)

SET(SEDA_LIBRARY_NAMES ${SEDA_LIBRARY_NAMES} libseda.a)
FIND_LIBRARY(SEDA_LIBRARY NAMES ${SEDA_LIBRARY_NAMES}
  HINTS
    ${PC_SEDA_LIBDIR}
    ${PC_SEDA_LIBRARY_DIRS}
    ${_seda_LIBRARIES_SEARCH_DIRS}
)

# if the include and the program are found then we have it
if(SEDA_LIBRARY AND SEDA_INCLUDE_DIRS) 
    message(STATUS "Found seda: -I${SEDA_INCLUDE_DIRS} ${SEDA_LIBRARY}")
    GET_FILENAME_COMPONENT (SEDA_LIBRARY_DIRS ${SEDA_LIBRARY} PATH)
    GET_FILENAME_COMPONENT (SEDA_LIBRARIES ${SEDA_LIBRARY} NAME)
    SET(SEDA_FOUND "YES")
endif(SEDA_LIBRARY AND SEDA_INCLUDE_DIRS)

MARK_AS_ADVANCED(
  SEDA_LIBRARY
  SEDA_LIBRARIES
  SEDA_LIBRARY_DIRS
  SEDA_INCLUDE_DIRS
)
