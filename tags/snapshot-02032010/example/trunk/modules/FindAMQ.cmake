# Finds the protocol buffers compiler, protoc.
# Protocol Buffers is available at http://code.google.com/apis/protocolbuffers
# This file defines:
# * AMQ_FOUND if protoc was found
# * AMQ_LIBRARY The lib to link to (currently only a static unix lib, not portable) 

IF( NOT AMQ_HOME )
  SET(AMQ_HOME $ENV{AMQ_HOME})
ENDIF(NOT AMQ_HOME)
message(STATUS "AMQ-Inc: ${AMQ_INCLUDE_DIRS}")

IF(WIN32 AND MSVC)
  SET(AMQ_INCLUDE_DIRS ${AMQ_HOME}/include/activemq-cpp-2.1.3)
ELSE(WIN32 AND MSVC)
  #FIND_PATH(AMQ_INCLUDE_DIRS activemq/core/ActiveMQSession.h
  FIND_PATH(AMQ_INCLUDE_DIRS Message.h
    ${AMQ_HOME}/include/activemq-cpp-2.1.3
    ${CMAKE_INCLUDE_PATH}
    /usr/local/include
    /usr/include
    PATH_SUFFIXES include activemq-cpp-2.1.3 include/activemq-cpp-2.1.3/cms)
ENDIF(WIN32 AND MSVC)

message(STATUS "AMQ-Inc: ${AMQ_INCLUDE_DIRS}")
message(STATUS "Libpath: ${CMAKE_LIBRARY_PATH}")
message(STATUS "INCpath: ${CMAKE_INCLUDE_PATH}")

# Support preference of static libs by adjusting CMAKE_FIND_LIBRARY_SUFFIXES
IF(WIN32)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ELSE(WIN32)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(WIN32)

SET(AMQ_LIBRARY_NAMES ${AMQ_LIBRARY_NAMES} libactivemq-cpp)

FIND_LIBRARY(AMQ_LIBRARY
  NAMES ${AMQ_LIBRARY_NAMES}
  PATHS ${CMAKE_LIBRARY_PATH} ${AMQ_HOME} $ENV{PATH} /usr/lib /usr/local/lib
  PATH_SUFFIXES lib
)
message(STATUS "AMQ-Lib: ${AMQ_LIBRARY}")
message(STATUS "AMQ-Inc: ${AMQ_INCLUDE_DIRS}")

# if the include and the program are found then we have it
IF(AMQ_LIBRARY) 
  SET(AMQ_FOUND "YES")
ENDIF(AMQ_LIBRARY)

MARK_AS_ADVANCED(
  AMQ_LIBRARY
  AMQ_INCLUDE_DIRS
)
