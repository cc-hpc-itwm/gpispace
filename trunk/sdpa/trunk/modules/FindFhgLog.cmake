# locates the FHG Logging framework
# This file defines:
# * FHGLOG_FOUND if fhglog was found
# * FHGLOG_LIBRARY The lib to link to (currently only a static unix lib) 
# * FHGLOG_INCLUDE_DIR

if(FHGLOG_HOME MATCHES "")
  if("" MATCHES "$ENV{FHGLOG_HOME}")
    message(STATUS "FHGLOG_HOME env is not set, setting it to /usr/local")
    set (FHGLOG_HOME "/usr/local")
  else("" MATCHES "$ENV{FHGLOG_HOME}")
    set (FHGLOG_HOME "$ENV{FHGLOG_HOME}")
  endif("" MATCHES "$ENV{FHGLOG_HOME}")
else(FHGLOG_HOME MATCHES "")
  message(STATUS "FHGLOG_HOME is not empty: \"${FHGLOG_HOME}\"")
  set (FHGLOG_HOME "${FHGLOG_HOME}")
endif(FHGLOG_HOME MATCHES "")

FIND_PATH(FHGLOG_INCLUDE_DIR fhglog/Logger.hpp
  ${FHGLOG_HOME}/include
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
)

IF(WIN32)
  SET(FHGLOG_LIBRARY_NAMES ${FHGLOG_LIBRARY_NAMES} libfhglog.lib)
ELSE(WIN32)
  SET(FHGLOG_LIBRARY_NAMES ${FHGLOG_LIBRARY_NAMES} libfhglog.a libfhglog.so)
ENDIF(WIN32)

FIND_LIBRARY(FHGLOG_LIBRARY
  NAMES ${FHGLOG_LIBRARY_NAMES}
  PATHS ${FHGLOG_HOME}/lib ${CMAKE_LIBRARY_PATH} /usr/lib /usr/local/lib
  )

# if the include and the program are found then we have it
IF(FHGLOG_INCLUDE_DIR AND FHGLOG_LIBRARY) 
  SET(FHGLOG_FOUND "YES")
  message(STATUS "Looking for FHGLog ... found headers in ${FHGLOG_INCLUDE_DIR} and library ${FHGLOG_LIBRARY}")
else(FHGLOG_INCLUDE_DIR AND FHGLOG_LIBRARY) 
  message(STATUS "FHGLog could not be found, try setting FHGLOG_HOME (value=\"${FHGLOG_HOME}\").")
ENDIF(FHGLOG_INCLUDE_DIR AND FHGLOG_LIBRARY)

MARK_AS_ADVANCED(
  FHGLOG_HOME
  FHGLOG_LIBRARY
  FHGLOG_INCLUDE_DIR
)
