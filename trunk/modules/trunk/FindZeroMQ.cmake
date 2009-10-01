# Finds the zero mq library
#
# ZeroMQ is available from http://www.zeromq.org
#
# This file defines:
# * ZMQ_FOUND if the library and include files were found
# * ZMQ_SERVER if the server could be found it's location is set here - not required for building
# * ZMQ_LIBRARY the library used to link to zmq (static is preferred)
# * ZMQ_INCLUDE_DIR the inlude directory for the zmq header files

# set defaults
SET(_zmq_HOME "/usr/local")
SET(_zmq_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_zmq_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  )

SET(_zmq_BINARY_SEARCH_DIRS
  ${CMAKE_BINARY_PATH}
  /usr/local/lib
  /usr/lib
  )

##
if( "${ZMQ_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{ZMQ_HOME}")
    message(STATUS "ZMQ_HOME env is not set, setting it to /usr/local")
    set (ZMQ_HOME ${_zmq_HOME})
  else("" MATCHES "$ENV{ZMQ_HOME}")
    set (ZMQ_HOME "$ENV{ZMQ_HOME}")
  endif("" MATCHES "$ENV{ZMQ_HOME}")
else( "${ZMQ_HOME}" STREQUAL "")
  message(STATUS "ZMQ_HOME is not empty: \"${ZMQ_HOME}\"")
endif( "${ZMQ_HOME}" STREQUAL "")
##

message(STATUS "Looking for zmq in ${ZMQ_HOME}")

IF( NOT ${ZMQ_HOME} STREQUAL "" )
    SET(_zmq_INCLUDE_SEARCH_DIRS ${ZMQ_HOME}/include ${_zmq_INCLUDE_SEARCH_DIRS})
    SET(_zmq_LIBRARIES_SEARCH_DIRS ${ZMQ_HOME}/lib ${_zmq_LIBRARIES_SEARCH_DIRS})
    SET(_zmq_BINARY_SEARCH_DIRS ${ZMQ_HOME}/bin ${_zmq_BINARY_SEARCH_DIRS})
    SET(_zmq_HOME ${ZMQ_HOME})
ENDIF( NOT ${ZMQ_HOME} STREQUAL "" )

IF( NOT $ENV{ZMQ_INCLUDEDIR} STREQUAL "" )
  SET(_zmq_INCLUDE_SEARCH_DIRS $ENV{ZMQ_INCLUDEDIR} ${_zmq_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{ZMQ_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{ZMQ_LIBRARYDIR} STREQUAL "" )
  SET(_zmq_LIBRARIES_SEARCH_DIRS $ENV{ZMQ_LIBRARYDIR} ${_zmq_LIBRARIES_SEARCH_DIRS})
ENDIF( NOT $ENV{ZMQ_LIBRARYDIR} STREQUAL "" )

IF( NOT $ENV{ZMQ_BINARYDIR} STREQUAL "" )
  SET(_zmq_BINARY_SEARCH_DIRS $ENV{ZMQ_BINARYDIR} ${_zmq_BINARY_SEARCH_DIRS})
ENDIF( NOT $ENV{ZMQ_BINARYDIR} STREQUAL "" )

IF( ZMQ_HOME )
  SET(_zmq_INCLUDE_SEARCH_DIRS ${ZMQ_HOME}/include ${_zmq_INCLUDE_SEARCH_DIRS})
  SET(_zmq_LIBRARIES_SEARCH_DIRS ${ZMQ_HOME}/lib ${_zmq_LIBRARIES_SEARCH_DIRS})
  SET(_zmq_BINARY_SEARCH_DIRS ${ZMQ_HOME}/lib ${_zmq_BINARY_SEARCH_DIRS})
  SET(_zmq_HOME ${ZMQ_HOME})
ENDIF( ZMQ_HOME )

# find the include files
FIND_PATH(ZMQ_INCLUDE_DIR
  NAMES  zmq/message.hpp
  PATHS ${_zmq_INCLUDE_SEARCH_DIRS}
  NO_DEFAULT_PATH
)

# locate the library
IF(WIN32)
  SET(ZMQ_LIBRARY_NAMES ${ZMQ_LIBRARY_NAMES} libzmq.lib)
ELSE(WIN32)
  SET(ZMQ_LIBRARY_NAMES ${ZMQ_LIBRARY_NAMES} libzmq.a libzmq.so)
ENDIF(WIN32)

FIND_LIBRARY(ZMQ_LIBRARY
  NAMES ${ZMQ_LIBRARY_NAMES}
  PATHS ${_zmq_LIBRARIES_SEARCH_DIRS}
  NO_DEFAULT_PATH
)

# try to locate the zmq_server
IF(WIN32)
  FIND_FILE(ZMQ_SERVER
    NAMES
    zmq_server.exe
    PATHS
    ${_zmq_BINARY_SEARCH_DIRS}
    DOC "Location of the zmq_server"
  )
ELSE(WIN32)
  FIND_FILE(ZMQ_SERVER
    NAMES
    zmq_server
    PATHS
    ${_zmq_BINARY_SEARCH_DIRS}
    DOC "Location of the zmq_server"
  )
ENDIF(WIN32)

if(ZMQ_SERVER)
  message(STATUS "ZeroMQ-server found at: ${ZMQ_SERVER}")
endif(ZMQ_SERVER)

# if the include and the program are found then we have it
if(ZMQ_LIBRARY AND ZMQ_INCLUDE_DIR)
  message(STATUS "Found ZeroMQ: I:${ZMQ_INCLUDE_DIR} L:${ZMQ_LIBRARY}")
  set(ZMQ_FOUND "YES")
else(ZMQ_LIBRARY AND ZMQ_INCLUDE_DIR)
  message(STATUS "ZeroMQ could not be found, try setting ZMQ_HOME (value=\"${_zmq_HOME}\").")
  set(ZMQ_FOUND "NO")
endif(ZMQ_LIBRARY AND ZMQ_INCLUDE_DIR)


MARK_AS_ADVANCED(
  ZMQ_HOME
  ZMQ_SERVER
  ZMQ_LIBRARY
  ZMQ_INCLUDE_DIR
)

