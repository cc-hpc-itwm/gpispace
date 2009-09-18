# Finds the protocol buffers compiler, protoc.
# Protocol Buffers is available at http://code.google.com/apis/protocolbuffers
# This file defines:
# * PB_FOUND if protoc was found
# * PB_LIBRARY The lib to link to (currently only a static unix lib, not
# portable) 
# * PB_PROTOC_CMD the protoc executable

# set defaults
SET(_pb_HOME "/usr/local")
SET(_pb_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_pb_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  )

##
if( "${PB_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{PB_HOME}")
    message(STATUS "PB_HOME env is not set, setting it to /usr/local")
    set (PB_HOME ${_pb_HOME})
  else("" MATCHES "$ENV{PB_HOME}")
    set (PB_HOME "$ENV{PB_HOME}")
  endif("" MATCHES "$ENV{PB_HOME}")
else( "${PB_HOME}" STREQUAL "")
  message(STATUS "PB_HOME is not empty: \"${PB_HOME}\"")
endif( "${PB_HOME}" STREQUAL "")
##

message(STATUS "Looking for protobuf in ${PB_HOME}")

IF( NOT ${PB_HOME} STREQUAL "" )
    SET(_pb_INCLUDE_SEARCH_DIRS ${PB_HOME}/include ${_pb_INCLUDE_SEARCH_DIRS})
    SET(_pb_LIBRARIES_SEARCH_DIRS ${PB_HOME}/lib ${_pb_LIBRARIES_SEARCH_DIRS})
    SET(_pb_HOME ${PB_HOME})
  ENDIF( NOT ${PB_HOME} STREQUAL "" )

  IF( NOT $ENV{PB_INCLUDEDIR} STREQUAL "" )
    SET(_pb_INCLUDE_SEARCH_DIRS $ENV{PB_INCLUDEDIR} ${_pb_INCLUDE_SEARCH_DIRS})
  ENDIF( NOT $ENV{PB_INCLUDEDIR} STREQUAL "" )

  IF( NOT $ENV{PB_LIBRARYDIR} STREQUAL "" )
    SET(_pb_LIBRARIES_SEARCH_DIRS $ENV{PB_LIBRARYDIR} ${_pb_LIBRARIES_SEARCH_DIRS})
  ENDIF( NOT $ENV{PB_LIBRARYDIR} STREQUAL "" )

  IF( PB_HOME )
    SET(_pb_INCLUDE_SEARCH_DIRS ${PB_HOME}/include ${_pb_INCLUDE_SEARCH_DIRS})
    SET(_pb_LIBRARIES_SEARCH_DIRS ${PB_HOME}/lib ${_pb_LIBRARIES_SEARCH_DIRS})
    SET(_pb_HOME ${PB_HOME})
  ENDIF( PB_HOME )

# find the include files
FIND_PATH(PB_INCLUDE_DIR
  NAMES  google/protobuf/message.h
 PATHS   ${_pb_INCLUDE_SEARCH_DIRS}
)


IF(WIN32)
  FIND_FILE(PB_PROTOC_CMD
    NAMES
    protoc.exe
    PATHS
    ${_pb_HOME}/bin
    ${CMAKE_BINARY_PATH}
    "[HKEY_CURRENT_USER\\protobuf\\bin]"
    /usr/local/bin
    /usr/bin
    PATH_SUFFIXES
    bin
    DOC "Location of the protoc file"
    )
ELSE(WIN32)
  FIND_FILE(PB_PROTOC_CMD
    NAMES
    protoc
    PATHS
    ${_pb_HOME}/bin
    ${CMAKE_BINARY_PATH}
    /usr/local/bin
    /usr/bin
    PATH_SUFFIXES
    bin
    DOC "Location of the protoc file"
    )
ENDIF(WIN32)

IF(WIN32)
  SET(PB_LIBRARY_NAMES ${PB_LIBRARY_NAMES} libprotobuf.lib)
ELSE(WIN32)
  SET(PB_LIBRARY_NAMES ${PB_LIBRARY_NAMES} libprotobuf.a libprotobuf.so)
ENDIF(WIN32)

FIND_LIBRARY(PB_LIBRARY
  NAMES ${PB_LIBRARY_NAMES}
  PATHS ${_pb_LIBRARIES_SEARCH_DIRS}
  )

# if the include and the program are found then we have it
IF(PB_PROTOC_CMD AND PB_LIBRARY) 
  SET(PB_FOUND "YES")
else(PB_PROTOC_CMD AND PB_LIBRARY) 
  message(STATUS "ProtocolBuffers could not be found, try setting PB_HOME (value=\"${PB_HOME}\").")
ENDIF(PB_PROTOC_CMD AND PB_LIBRARY)

MARK_AS_ADVANCED(
  PB_HOME
  PB_PROTOC_CMD
  PB_LIBRARY
  PB_INCLUDE_DIR
  )

