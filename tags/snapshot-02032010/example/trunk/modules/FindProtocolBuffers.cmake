# -*- mode: cmake; -*-
# Finds the protocol buffers compiler, protoc.
# Protocol Buffers is available at http://code.google.com/apis/protocolbuffers
# This file defines:
# * PB_FOUND if protoc was found
# * PB_LIBRARY The lib to link to (currently only a static unix lib, not
# portable) 
# * PB_PROTOC_CMD the protoc executable

cmake_minimum_required(VERSION "2.6.2" FATAL_ERROR)
message(STATUS "FindProtocolBuffers check")

include(FindPackageHelper)
check_package_search_path(PB google/protobuf/message.h protobuf 2.0.3
  PB_INCLUDE_SEARCH_DIRS
  PB_LIBRARIES_SEARCH_DIRS
  PB_BINARY_SEARCH_DIRS
  PB_HOME)

check_package(PB google/protobuf/message.h protobuf 2.0.3)

IF(WIN32)
  FIND_FILE(PB_PROTOC_CMD
    NAMES    protoc.exe
    HINTS    ${PB_BINARY_SEARCH_DIRS}
    "[HKEY_CURRENT_USER\\protobuf\\bin]"
    DOC "Location of the protoc file"
    )
ELSE(WIN32)
  FIND_FILE(PB_PROTOC_CMD
    NAMES    protoc
    HINTS    ${PB_BINARY_SEARCH_DIRS}
    DOC "Location of the protoc file"
    )
ENDIF(WIN32)

# if the include and the program are found then we have it
IF(PB_PROTOC_CMD AND PB_LIBRARY) 
  SET(PB_FOUND "YES")
  message(STATUS "Found ProtocolBuffers Inc:${PB_INCLUDE_DIR} Lib:${PB_LIBRARY} Cmd:${PB_PROTOC_CMD}")
else(PB_PROTOC_CMD AND PB_LIBRARY) 
  message(STATUS "ProtocolBuffers could not be found, try setting PB_HOME (value=\"${PB_HOME}\").")
ENDIF(PB_PROTOC_CMD AND PB_LIBRARY)

MARK_AS_ADVANCED(
#  PB_HOME
#  PB_FOUND
  PB_PROTOC_CMD
#  PB_LIBRARY
#  PB_LIBRARIES
#  PB_LIBRARY_DIR
#  PB_INCLUDE_DIR
  )

