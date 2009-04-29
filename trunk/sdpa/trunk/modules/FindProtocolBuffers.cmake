# Finds the protocol buffers compiler, protoc.
# Protocol Buffers is available at http://code.google.com/apis/protocolbuffers
# This file defines:
# * PB_FOUND if protoc was found
# * PB_LIBRARY The lib to link to (currently only a static unix lib, not
# portable) 
# * PB_PROTOC_CMD the protoc executable

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

FIND_PATH(PB_INCLUDE_DIR google/protobuf/message.h
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
)

SET(PB_LIBRARY_NAMES ${PB_LIBRARY_NAMES} libprotobuf.a)
FIND_LIBRARY(PB_LIBRARY
  NAMES ${PB_LIBRARY_NAMES}
  PATHS ${CMAKE_LIBRARY_PATH} /usr/lib /usr/local/lib
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
