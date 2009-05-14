# Finds the protocol buffers compiler, protoc.
# Protocol Buffers is available at http://code.google.com/apis/protocolbuffers
# This file defines:
# * PB_FOUND if protoc was found
# * PB_LIBRARY The lib to link to (currently only a static unix lib, not
# portable) 
# * PB_PROTOC_CMD the protoc executable

IF( NOT PB_ROOT )
  SET(PB_ROOT "")
ENDIF(NOT PB_ROOT)

IF(WIN32)
FIND_FILE(PB_PROTOC_CMD
  NAMES
    protoc.exe
  PATHS
    "[HKEY_CURRENT_USER\\smc\\bin]"
    /usr/local/bin
    /usr/bin
    $ENV{PROTOC_HOME}/bin
    ${PB_ROOT}/bin
    PATH_SUFFIXES
    bin
  DOC "Location of the protoc file"
)
ELSE(WIN32)
FIND_FILE(PB_PROTOC_CMD
  NAMES
    protoc
  PATHS
    "[HKEY_CURRENT_USER\\smc\\bin]"
    /usr/local/bin
    /usr/bin
    $ENV{PROTOC_HOME}/bin
    PATH_SUFFIXES
    bin
  DOC "Location of the protoc file"
)
ENDIF(WIN32)

FIND_PATH(PB_INCLUDE_DIR google/protobuf/message.h
  ${CMAKE_INCLUDE_PATH}
  $ENV{PB_HOME}/include
  ${PB_ROOT}/include
  /usr/local/include
  /usr/include
)

IF(WIN32)
  SET(PB_LIBRARY_NAMES ${PB_LIBRARY_NAMES} libprotobuf.lib)
ELSE(WIN32)
  SET(PB_LIBRARY_NAMES ${PB_LIBRARY_NAMES} libprotobuf.a)
ENDIF(WIN32)

FIND_LIBRARY(PB_LIBRARY
  NAMES ${PB_LIBRARY_NAMES}
  PATHS ${CMAKE_LIBRARY_PATH} ${PB_ROOT}/lib /usr/lib /usr/local/lib
)


# if the include and the program are found then we have it
IF(PB_PROTOC_CMD AND PB_LIBRARY) 
  SET(PB_FOUND "YES")
ENDIF(PB_PROTOC_CMD AND PB_LIBRARY)

MARK_AS_ADVANCED(
  PB_PROTOC_CMD
  PB_LIBRARY
  PB_INCLUDE_DIR
)

