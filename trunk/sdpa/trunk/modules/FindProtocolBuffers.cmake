# Finds the protocol buffers compiler, protoc.
# Protocol Buffers is available at http://code.google.com/apis/protocolbuffers
# This file defines:
# * PB_FOUND if protoc was found
# * PB_LIBRARY The lib to link to (currently only a static unix lib, not
# portable) 
# * PB_PROTOC_CMD the protoc executable

if(PB_HOME MATCHES "")
  if("" MATCHES "$ENV{PB_HOME}")
    message(STATUS "PB_HOME env is not set, setting it to /usr/local")
    set (PB_HOME "/usr/local")
  else("" MATCHES "$ENV{PB_HOME}")
    set (PB_HOME "$ENV{PB_HOME}")
  endif("" MATCHES "$ENV{PB_HOME}")
else(PB_HOME MATCHES "")
  message(STATUS "PB_HOME is not empty: \"${PB_HOME}\"")
  set (PB_HOME "${PB_HOME}")
endif(PB_HOME MATCHES "")

IF(WIN32)
  FIND_FILE(PB_PROTOC_CMD
    NAMES
    protoc.exe
    PATHS
    ${PB_HOME}/bin
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
    ${PB_HOME}/bin
    ${CMAKE_BINARY_PATH}
    /usr/local/bin
    /usr/bin
    PATH_SUFFIXES
    bin
    DOC "Location of the protoc file"
    )
ENDIF(WIN32)

FIND_PATH(PB_INCLUDE_DIR google/protobuf/message.h
  ${PB_HOME}/include
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

IF(WIN32)
  SET(PB_LIBRARY_NAMES ${PB_LIBRARY_NAMES} libprotobuf.lib)
ELSE(WIN32)
  SET(PB_LIBRARY_NAMES ${PB_LIBRARY_NAMES} libprotobuf.a libprotobuf.so)
ENDIF(WIN32)

FIND_LIBRARY(PB_LIBRARY
  NAMES ${PB_LIBRARY_NAMES}
  PATHS ${PB_HOME}/lib ${CMAKE_LIBRARY_PATH} /usr/lib /usr/local/lib
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

