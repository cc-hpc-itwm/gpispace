# Tries to locate a logging framework.
# This file defines:
# * SEDA_FOUND if log4cpp was found
# * SEDA_LIBRARY The lib to link to (currently only a static unix lib, not portable) 
# * SEDA_INCLUDE_DIR The path to the include directory

FIND_PATH(SEDA_INCLUDE_DIR seda/IEvent.hpp
  ${CMAKE_INCLUDE_PATH}
  $ENV{SEDA_HOME}/include
  /usr/local/include
  /usr/include
)

SET(SEDA_LIBRARY_NAMES ${SEDA_LIBRARY_NAMES} libseda.a)
FIND_LIBRARY(SEDA_LIBRARY
  NAMES ${SEDA_LIBRARY_NAMES}
  PATHS ${CMAKE_LIBRARY_PATH} $ENV{SEDA_HOME}/lib /usr/lib /usr/local/lib
)


# if the include and the program are found then we have it
IF(SEDA_LIBRARY AND SEDA_INCLUDE_DIR) 
  message(STATUS "Found libseda: -I${SEDA_INCLUDE_DIR} -l${SEDA_LIBRARY}")
  SET(SEDA_FOUND "YES")
ENDIF(SEDA_LIBRARY AND SEDA_INCLUDE_DIR)

MARK_AS_ADVANCED(
  SEDA_LIBRARY
  SEDA_INCLUDE_DIR
)
